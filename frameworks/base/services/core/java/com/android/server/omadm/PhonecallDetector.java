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

import java.util.Date;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.server.OmadmService;

public class PhonecallDetector {
    private final static String TAG = "PhonecallDetector";

    private OmadmService mOmadmService = null;
    private BroadcastReceiver mVoiceCallReceiver = null;

    // is actual for active voice call
    protected final static int NETWORK_ACTIVE_VOICE_CALL = 3;

    /**
     * Constructor.
     *
     * @param context      is Context the PhonecallDetector is running in.
     * @param omadmService is OmadmService for notification about Voice Call status.
     */
    public PhonecallDetector(Context context, OmadmService omadmService) {
        mOmadmService = omadmService;
    }

    /**
     * Starts telephony events detecting.
     *
     * @param context is Context the VoiceCallReceiver is running in.
     */
    public void register(Context context) {
        sendDefaultStateNotification(context);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_NEW_OUTGOING_CALL);
        mVoiceCallReceiver = new VoiceCallReceiver();
        context.registerReceiver(mVoiceCallReceiver, intentFilter);
    }

    /**
     * Stops telephony events detecting.
     *
     * @param context is Context the VoiceCallReceiver is running in.
     */
    public void unregister(Context context) {
        if (mVoiceCallReceiver != null) {
            context.unregisterReceiver(mVoiceCallReceiver);
            mVoiceCallReceiver = null;
        }
    }

    private void sendDefaultStateNotification(Context context) {
        TelephonyManager telephonyManager =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager == null) {
            Log.e(TAG, "sendDefaultStateNotification() TelephonyManager system service is null");
            return;
        }
        int state = telephonyManager.getCallState();
        switch (state) {
            case TelephonyManager.CALL_STATE_RINGING:
            case TelephonyManager.CALL_STATE_OFFHOOK:
                if (mOmadmService != null) {
                    mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, true);
                } else {
                    Log.e(TAG, "sendDefaultStateNotification() OmadmService object is null");
                }
                break;
            case TelephonyManager.CALL_STATE_IDLE:
                if (mOmadmService != null) {
                    mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, false);
                } else {
                    Log.e(TAG, "sendDefaultStateNotification() OmadmService object is null");
                }
                break;
        }
    }

    private class VoiceCallReceiver extends PhonecallReceiver {

        @Override
        protected void onIncomingCallReceived(Context ctx, String number, Date start)
        {
            Log.d(TAG, "onIncomingCallReceived() number: " + number +
                    " start at: " + start.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, true);
            } else {
                Log.e(TAG, "onIncomingCallReceived() OmadmService object is null");
            }
        }

        @Override
        protected void onIncomingCallAnswered(Context ctx, String number, Date start)
        {
            Log.d(TAG, "onIncomingCallAnswered() number: " + number +
                    " start at: " + start.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, true);
            } else {
                Log.e(TAG, "onIncomingCallAnswered() OmadmService object is null");
            }
        }

        @Override
        protected void onIncomingCallEnded(Context ctx, String number, Date start, Date end)
        {
            Log.d(TAG, "onIncomingCallEnded() number: " + number +
                    " start at: " + start.toString() + " end at: " + end.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, false);
            } else {
                Log.e(TAG, "onIncomingCallEnded() OmadmService object is null");
            }
        }

        @Override
        protected void onOutgoingCallStarted(Context ctx, String number, Date start)
        {
            Log.d(TAG, "onOutgoingCallStarted() number: " + number +
                    " start at: " + start.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, true);
            } else {
                Log.e(TAG, "onOutgoingCallStarted() OmadmService object is null");
            }
        }

        @Override
        protected void onOutgoingCallEnded(Context ctx, String number, Date start, Date end)
        {
            Log.d(TAG, "onOutgoingCallEnded() number: " + number +
                    " start at: " + start.toString() + " end at: " + end.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, false);
            } else {
                Log.e(TAG, "onOutgoingCallEnded() OmadmService object is null");
            }
        }

        @Override
        protected void onMissedCall(Context ctx, String number, Date start)
        {
            Log.d(TAG, "onMissedCall() number: " + number +
                    " start at: " + start.toString());
            if (mOmadmService != null) {
                mOmadmService.notifyOmadmNetworkManager(NETWORK_ACTIVE_VOICE_CALL, false);
            } else {
                Log.e(TAG, "onMissedCall() OmadmService object is null");
            }
        }
    }
}
