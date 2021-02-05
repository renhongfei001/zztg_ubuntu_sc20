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

package com.android.server.omadm;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.server.OmadmService;

/**
 * Class is intended to track network state changes.
 */
public class NetworkDetector {
    private final static String TAG = "NetworkDetector";

    private final static int NETWORK_WIFI_CONNECTED     = 0;
    // Determine the status of ADMIN apn with FOTA Capabilities
    // for any cellular data connection such as LTE or 3G and etc.
    private final static int NETWORK_ADMINAPN_CONNECTED = 1;
    private final static int NETWORK_DATA_ROAMING       = 2;

    private static boolean mActiveVoiceCall = false;

    private final OmadmService mOmadmService;

    private TelephonyManager mTelephonyManager = null;
    private ConnectivityManager mConnectivityManager = null;
    private BroadcastReceiver mNetEventsReceiver = null;

    /**
     * Constructor.
     *
     * @param context      is Context the NetworkDetector is running in.
     * @param omadmService is OmadmService for which network connection changes
     *                     are tracked.
     */
    public NetworkDetector(Context context, OmadmService omadmService) {
        mOmadmService = omadmService;
        mTelephonyManager =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        mConnectivityManager =
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    /**
     * Starts network events detecting. It internally registers all network receivers
     * and listeners for network tracking.
     *
     * @param context is Context the NetworkDetector is running in.
     */
    public void register(Context context) {
        sendDefaultStateNotification();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mNetEventsReceiver = new NetworkEventsBroadcastReceiver();
        context.registerReceiver(mNetEventsReceiver, intentFilter);
    }

    /**
     * Stops network events detecting.
     *
     * @param context is Context the NetworkDetector is running in.
     */
    public void unregister(Context context) {
        if (mNetEventsReceiver != null) {
            context.unregisterReceiver(mNetEventsReceiver);
            mNetEventsReceiver = null;
        }
    }

    private void sendDefaultStateNotification() {
        if (mConnectivityManager == null) {
            Log.e(TAG, "sendDefaultStateNotification() ConnectivityManager system service is null");
            return;
        }
        NetworkInfo networkInfo = mConnectivityManager.getActiveNetworkInfo();
        Feature feature = Feature.getFeature(networkInfo, mTelephonyManager.getPhoneType());
        int features[] = {
                NETWORK_WIFI_CONNECTED, NETWORK_ADMINAPN_CONNECTED, NETWORK_DATA_ROAMING
        };
        for (int type : features) {
            if (feature != null && type == feature.type) {
                Log.d(TAG, "Notify omadm NM with type: " + feature.type +
                        ", status: " + feature.status);
                if (NETWORK_ADMINAPN_CONNECTED == feature.type) {
                    // Update cellular data roaming state
                    mOmadmService.notifyOmadmNetworkManager(NETWORK_DATA_ROAMING,
                            networkInfo.isRoaming());
                }
                mOmadmService.notifyOmadmNetworkManager(feature.type, feature.status);
            } else {
                Log.d(TAG, "Notify omadm NM with type: " + type + ", status: false");
                mOmadmService.notifyOmadmNetworkManager(type, false);
            }
        }
    }

    private void networkChanged(NetworkInfo networkInfo) {
        Log.d(TAG, "networkChanged()");
        if (mTelephonyManager == null) {
            Log.e(TAG, "onReceive() TelephonyManager system service is null");
            return;
        }
        int type = networkInfo.getType();
        Log.d(TAG, "onReceive() - handle for type: " + type);
        Feature feature = Feature.getFeature(networkInfo, mTelephonyManager.getPhoneType());
        if (networkInfo != null && feature != null) {
            if (NETWORK_ADMINAPN_CONNECTED == feature.type) {
                // Update cellular data roaming state
                mOmadmService.notifyOmadmNetworkManager(NETWORK_DATA_ROAMING,
                        mTelephonyManager.isNetworkRoaming());
            }
            // Update connection state
            mOmadmService.notifyOmadmNetworkManager(feature.type, feature.status);
        } else {
            Log.d(TAG, "onReceive() - nothing to handle for type: " + type);
        }
    }

    /**
     * Internal class to track network broadcasts.
     */
    private class NetworkEventsBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "NetworkEventsBroadcastReceiver onReceive()");

            NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(ConnectivityManager
                    .EXTRA_NETWORK_INFO);
            networkChanged(networkInfo);
        }
    }

    private static class Feature {
        public int type;
        public boolean status;

        public Feature(int type, boolean status) {
            this.type = type;
            this.status = status;
        }

        public static Feature getFeature(NetworkInfo networkInfo, int phoneType) {
            if (null == networkInfo) {
                Log.d(TAG, "networkInfo is null");
                return null;
            }
            int type = networkInfo.getType();
            Log.d(TAG, "Phone type: " + phoneType + " Network type: " + type +
                    " NetworkInfo content: " + networkInfo.toString());
            switch (type) {
                case ConnectivityManager.TYPE_MOBILE_FOTA:
                    return new Feature(NETWORK_ADMINAPN_CONNECTED, networkInfo.isConnected());
                case ConnectivityManager.TYPE_MOBILE:
                    /*
                     * We separated the phone type CDMA from GSM
                     * - CDMA phone type is for Verizon devices which have admin apn
                     * and handled by TYPE_MOBILE_FOTA connection type
                     * - GSM  phone type is for other devices which is temporarily used for testing
                     * TYPE_MOBILE case should be removed by OEM in final release.
                     */
                    switch (phoneType) {
                        case TelephonyManager.PHONE_TYPE_CDMA:
                            // Do nothing for CDMA phone type like Verizon devices
                            break;
                        case TelephonyManager.PHONE_TYPE_GSM:
                            return new Feature(NETWORK_ADMINAPN_CONNECTED,
                                    networkInfo.isConnected());
                    }
                    break;
                case ConnectivityManager.TYPE_WIFI:
                    return new Feature(NETWORK_WIFI_CONNECTED, networkInfo.isConnected());
            }
            return null;
        }
    }
}
