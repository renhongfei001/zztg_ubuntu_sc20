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
import android.telephony.TelephonyManager;

public abstract class PhonecallReceiver extends BroadcastReceiver {

    //The receiver will be recreated whenever android feels like it.
    //We need a static variable to remember data between instantiations

    private static int mLastState = TelephonyManager.CALL_STATE_IDLE;
    private static Date mCallStartTime;
    private static boolean mIsIncoming;
    private static String mSavedNumber;  //because the passed incoming is only valid in ringing

    @Override
    public void onReceive(Context context, Intent intent) {

        //We listen to two intents. The new outgoing call only tells us of an outgoing call.
        //We use it to get the number.
        if (intent.getAction().equals(Intent.ACTION_NEW_OUTGOING_CALL)) {
            mSavedNumber = intent.getExtras().getString(Intent.EXTRA_PHONE_NUMBER);
        } else {
            String stateStr = intent.getExtras().getString(TelephonyManager.EXTRA_STATE);
            String number = intent.getExtras().getString(TelephonyManager.EXTRA_INCOMING_NUMBER);
            int state = 0;
            if(stateStr.equals(TelephonyManager.EXTRA_STATE_IDLE)) {
                state = TelephonyManager.CALL_STATE_IDLE;
            } else if (stateStr.equals(TelephonyManager.EXTRA_STATE_OFFHOOK)) {
                state = TelephonyManager.CALL_STATE_OFFHOOK;
            } else if (stateStr.equals(TelephonyManager.EXTRA_STATE_RINGING)) {
                state = TelephonyManager.CALL_STATE_RINGING;
            }

            onCallStateChanged(context, state, number);
        }
    }

    //Derived classes should override these to respond to specific events of interest
    protected abstract void onIncomingCallReceived(Context ctx, String number, Date start);
    protected abstract void onIncomingCallAnswered(Context ctx, String number, Date start);
    protected abstract void onIncomingCallEnded(Context ctx, String number, Date start, Date end);

    protected abstract void onOutgoingCallStarted(Context ctx, String number, Date start);
    protected abstract void onOutgoingCallEnded(Context ctx, String number, Date start, Date end);

    protected abstract void onMissedCall(Context ctx, String number, Date start);

    //Deals with actual events

    //Incoming call - goes from IDLE to RINGING when it rings, to OFFHOOK when it's answered,
    //to IDLE when its hung up
    //Outgoing call - goes from IDLE to OFFHOOK when it dials out, to IDLE when hung up
    public void onCallStateChanged(Context context, int state, String number) {
        if(mLastState == state) {
            //No change, debounce extras
            return;
        }
        switch (state) {
            case TelephonyManager.CALL_STATE_RINGING:
                mIsIncoming = true;
                mCallStartTime = new Date();
                mSavedNumber = number;
                onIncomingCallReceived(context, number, mCallStartTime);
                break;
            case TelephonyManager.CALL_STATE_OFFHOOK:
                //Transition of ringing->offhook are pickups of incoming calls.  Nothing done on them
                if(mLastState != TelephonyManager.CALL_STATE_RINGING) {
                    mIsIncoming = false;
                    mCallStartTime = new Date();
                    onOutgoingCallStarted(context, mSavedNumber, mCallStartTime);
                } else {
                    mIsIncoming = true;
                    mCallStartTime = new Date();
                    onIncomingCallAnswered(context, mSavedNumber, mCallStartTime);
                }
                break;
            case TelephonyManager.CALL_STATE_IDLE:
                //Went to idle - this is the end of a call.  What type depends on previous state(s)
                if(mLastState == TelephonyManager.CALL_STATE_RINGING) {
                    //Ring but no pickup - a miss
                    onMissedCall(context, mSavedNumber, mCallStartTime);
                } else if(mIsIncoming) {
                    onIncomingCallEnded(context, mSavedNumber, mCallStartTime, new Date());
                } else {
                    onOutgoingCallEnded(context, mSavedNumber, mCallStartTime, new Date());
                }
                break;
        }
        mLastState = state;
    }
}
