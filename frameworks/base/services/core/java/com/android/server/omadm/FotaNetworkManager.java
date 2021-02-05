/*
 * Copyright (C) 2016 Verizon. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server.omadm;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.SystemClock;
import android.util.Log;
import com.android.server.omadm.FotaNetworkException;
import android.os.IBinder;
import com.android.server.OmadmService;
import com.android.server.omadm.PalConstants;

/**
 * Manages the MMS network connectivity
 */
public class FotaNetworkManager {
    // Timeout used to call ConnectivityManager.requestNetwork
    // TODO: temporarily reduced from 60 sec to 15 sec for test switching to
    // VZWADMIN apn outside of Verizon.
    private static final int NETWORK_REQUEST_TIMEOUT_MILLIS = 60 * 1000;
    // Wait timeout for this class, a little bit longer than the above timeout
    // to make sure we don't bail prematurely
    private static final int NETWORK_ACQUIRE_TIMEOUT_MILLIS =
            NETWORK_REQUEST_TIMEOUT_MILLIS + (5 * 1000);

    private static final String TAG = "FotaNetworkManager";

    private final Context mContext;

    // The requested Fota {@link android.net.Network} we are holding
    // We need this when we unbind from it. This is also used to indicate if the
    // Fota network is available.
    private Network mNetwork;
    // The current count of Fota requests that require the Fota network
    // If mFotaRequestCount is 0, we should release the Fota network.
    private int mFotaRequestCount;
    // This is really just for using the capability
    private final NetworkRequest mNetworkRequest;
    // The callback to register when we request Fota network
    private ConnectivityManager.NetworkCallback mNetworkCallback;

    private volatile ConnectivityManager mConnectivityManager;


    // The SIM ID which we use to connect
    private final int mSubId;

    // Parent service reference
    OmadmService mOmadmSrv = null;

    /**
     * Network callback for our network request
     */
    private class NetworkRequestCallback extends ConnectivityManager.NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            Log.i(TAG, "NetworkCallbackListener.onAvailable: network=" + network);
            synchronized (FotaNetworkManager.this) {
                mNetwork = network;
                FotaNetworkManager.this.notifyAll();
            }
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            Log.w(TAG, "NetworkCallbackListener.onLost: network=" + network);
            synchronized (FotaNetworkManager.this) {
                releaseRequestLocked(this);
                FotaNetworkManager.this.notifyAll();
                if (mOmadmSrv != null) {
                    mOmadmSrv.omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_LOST, 0);
                }
                mNetwork = null;
            }
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            Log.w(TAG, "NetworkCallbackListener.onUnavailable");
            synchronized (FotaNetworkManager.this) {
                releaseRequestLocked(this);
                FotaNetworkManager.this.notifyAll();
            }
        }
    }

    public FotaNetworkManager(IBinder service, Context context, int subId) {
        mContext = context;
        mNetworkCallback = null;
        mNetwork = null;
        mFotaRequestCount = 0;
        mConnectivityManager = null;
        mSubId = subId;
        if( service instanceof OmadmService ) {
            mOmadmSrv = (OmadmService)service;
        }
        Log.d("xxxx", "FotaNetworkManager:mSubId" +mSubId); 
        mNetworkRequest = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_FOTA)
                .setNetworkSpecifier(Integer.toString(mSubId))
                .build();
    }

    /**
     * Acquire the Fota network
     *
     * @param requestId request ID for logging
     * @return network ID
     * @throws FotaNetworkException if we fail to acquire it
     */
    public int acquireNetwork(final String callerID) throws FotaNetworkException {
        synchronized (this) {
            mFotaRequestCount += 1;
            if (mNetwork != null) {
                // Already available
                Log.d(callerID, "FotaNetworkManager: already available");
                return mNetwork.netId;
            }
            // Not available, so start a new request if not done yet
            if (mNetworkCallback == null) {
                Log.d(callerID, "FotaNetworkManager: start new network request");
                startNewNetworkRequestLocked();
            }
            final long shouldEnd = SystemClock.elapsedRealtime() + NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            long waitTime = NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            while (waitTime > 0) {
                try {
                    this.wait(waitTime);
                } catch (InterruptedException e) {
                    Log.w(callerID, "FotaNetworkManager: acquire network wait interrupted");
                }
                if (mNetwork != null) {
                    // Success
                    return mNetwork.netId;
                }
                // Calculate remaining waiting time to make sure we wait the full timeout period
                waitTime = shouldEnd - SystemClock.elapsedRealtime();
            }
            // Timed out, so release the request and fail
            Log.e(callerID, "FotaNetworkManager: timed out");
            releaseRequestLocked(mNetworkCallback);
            throw new FotaNetworkException("Acquiring network timed out");
        }
    }

    /**
     * Release the Fota network when nobody is holding on to it.
     *
     * @param callerID request ID for logging
     * @return is network released
     */
    public boolean releaseNetwork(final String callerID) {
        synchronized (this) {
            if (mFotaRequestCount > 0) {
                mFotaRequestCount -= 1;
                Log.d(callerID, "FotaNetworkManager: release, count=" + mFotaRequestCount);
                if (mFotaRequestCount < 1) {
                    Log.d(callerID, "FotaNetworkManager: Release FOTA Network");
                    releaseRequestLocked(mNetworkCallback);
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * Start a new {@link android.net.NetworkRequest} for Fota
     */
    private void startNewNetworkRequestLocked() {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        mNetworkCallback = new NetworkRequestCallback();
        connectivityManager.requestNetwork(
                mNetworkRequest, mNetworkCallback, NETWORK_REQUEST_TIMEOUT_MILLIS);
    }

    /**
     * Release the current {@link android.net.NetworkRequest} for Fota
     *
     * @param callback the {@link android.net.ConnectivityManager.NetworkCallback} to unregister
     */
    private void releaseRequestLocked(ConnectivityManager.NetworkCallback callback) {
        if (callback != null) {
            final ConnectivityManager connectivityManager = getConnectivityManager();
            try {
				if (mOmadmSrv != null) {
					Log.i(TAG,"releaseRequestLocked dispacth admin net status");
                    mOmadmSrv.omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_LOST, 0);  //unbind the netid in socket when release admin network``
                }
                connectivityManager.unregisterNetworkCallback(callback);
            } catch (IllegalArgumentException e) {
                // It is possible ConnectivityManager.requestNetwork may fail silently due
                // to RemoteException. When that happens, we may get an invalid
                // NetworkCallback, which causes an IllegalArgumentexception when we try to
                // unregisterNetworkCallback. This exception in turn causes
                // FotaNetworkManager to skip resetLocked() in the below. Thus Fota service
                // would get stuck in the bad state until the device restarts. This fix
                // catches the exception so that state clean up can be executed.
                Log.w(TAG, "Unregister network callback exception", e);
            }
        }
        resetLocked();
    }

    /**
     * Reset the state
     */
    private void resetLocked() {
        mNetworkCallback = null;
        mNetwork = null;
        mFotaRequestCount = 0;
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }


    /**
     * Get the APN name for the active network
     *
     * @return The APN name if available, otherwise null
     */
    public String getApnName() {
        Network network = null;
        synchronized (this) {
            if (mNetwork == null) {
                return null;
            }
            network = mNetwork;
        }
        String apnName = null;
        final ConnectivityManager connectivityManager = getConnectivityManager();
        final NetworkInfo FotaNetworkInfo = connectivityManager.getNetworkInfo(network);
        if (FotaNetworkInfo != null) {
            apnName = FotaNetworkInfo.getExtraInfo();
        }
        return apnName;
    }
}
