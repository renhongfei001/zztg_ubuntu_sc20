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

import android.content.Context;
import android.os.AsyncTask;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.Looper;
import android.telephony.SubscriptionManager;
import android.util.Slog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import java.util.concurrent.Semaphore;

public class AsyncTaskWorker extends AsyncTask<Integer, Void, String> {
    private static final String TAG = AsyncTaskWorker.class.getSimpleName();
    private static boolean DEBUG = ImsConfigManager.DEBUG;
    private static Context mContext = null;
    private static Semaphore mTaskLocker = null;
    private static final int COUNT_DOWN_TIMER_TIMEOUT = 15000;
    private static final int COUNT_DOWN_TIMER_INTERVAL = 15000;
    private static int errCodeAsyncTask = PalConstants.RESULT_ERROR;
    private static CountDownTimer taskTimer = null;
    private final AsyncTask asyncObject = this;
    private Handler mHandler = new Handler(Looper.getMainLooper());
    private static Semaphore timerReadyLock = new Semaphore(1);

    public AsyncTaskWorker (Context ctx, Semaphore taskLocker) {
        mContext = ctx;
        mTaskLocker = taskLocker;
        try {
            if (DEBUG) Slog.d(TAG, "try lock timer ready semaphore");
            timerReadyLock.acquire();
        } catch (InterruptedException e) {
            Slog.w(TAG, "InterruptedException occured");
            e.printStackTrace();
        }
        mHandler.post(new Runnable() {
            public void run() {
                if (DEBUG) Slog.d(TAG, "start taskTimer");
                taskTimer = new CountDownTimer(COUNT_DOWN_TIMER_TIMEOUT,
                            COUNT_DOWN_TIMER_INTERVAL) {
                    public void onFinish() {
                        if (DEBUG) Slog.d(TAG, "CountDownTimer finished");
                        // stop async task if not in progress
                        if (asyncObject.getStatus() == AsyncTask.Status.RUNNING) {
                            if (DEBUG) Slog.d(TAG, "task is in progress, stop it");
                            asyncObject.cancel(true);
                            ImsConfigManager.setErrCode(PalConstants.RESULT_ERROR_TIMEOUT);
                            ImsConfigManager.setResultValue("");
                        }
                        mTaskLocker.release();
                        if (DEBUG) Slog.d(TAG, "release task semaphore");
                    }
                    public void onTick(long tick) {}
                };
                if (DEBUG) Slog.d(TAG, "taskTimer is: " + taskTimer);
                timerReadyLock.release();
                if (DEBUG) Slog.d(TAG, "release timer ready semaphore");
            }
        });
    }

    @Override
    protected String doInBackground(Integer... params) {
        if (DEBUG) Slog.d(TAG, "doInBackground");
        String retVal = "";
        int count = params.length;
        if (DEBUG) Slog.d(TAG, "params count is: " + count);
        if (count > 0) {
            if (null == mContext) {
                errCodeAsyncTask = PalConstants.RESULT_ERROR;
                return "";
            }
            ImsManager imsManager = ImsManager.getInstance(
                    mContext, SubscriptionManager.getDefaultVoicePhoneId());
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig == null) {
                    Slog.w(TAG, "getImsDomain: imsConfig is null");
                    PalConstants.throwErrStatus(
                            PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
                }
                if (DEBUG) Slog.d(TAG, "try getProvisionedStringValue for: " + params[0].intValue());
                retVal = imsConfig.getProvisionedStringValue(params[0].intValue());
                errCodeAsyncTask = PalConstants.RESULT_SUCCESS;
            } catch (ImsException e) {
                Slog.w(TAG, "ImsException occured");
                errCodeAsyncTask = PalConstants.RESULT_ERROR;
                e.printStackTrace();
            }
        }
        return retVal;
    }

    @Override
    protected void onPreExecute() {
        if (DEBUG) Slog.d(TAG, "onPreExecute");
        try {
            if (DEBUG) Slog.d(TAG, "try lock timer ready semaphore");
            timerReadyLock.acquire();
            if (DEBUG) Slog.d(TAG, "try lock task semaphore");
            mTaskLocker.acquire();
        } catch (InterruptedException e) {
            Slog.w(TAG, "InterruptedException occured");
            e.printStackTrace();
        }
        if (DEBUG) Slog.d(TAG, "taskTimer is: " + taskTimer);
        taskTimer.start();
        timerReadyLock.release();
        if (DEBUG) Slog.d(TAG, "release timer ready semaphore");
    }

    @Override
    protected void onPostExecute(String result) {
        if (DEBUG) Slog.d(TAG, "onPostExecute: task finished with result: " + result +
                ", and error code: " + errCodeAsyncTask);
        if (PalConstants.RESULT_SUCCESS == errCodeAsyncTask) {
            ImsConfigManager.setResultValue(result);
        } else {
            ImsConfigManager.setResultValue("");
        }
        ImsConfigManager.setErrCode(errCodeAsyncTask);
        if (null != taskTimer) {
            taskTimer.cancel();
            if (DEBUG) Slog.d(TAG, "cancel task timer");
            taskTimer = null;
        }
        mTaskLocker.release();
        if (DEBUG) Slog.d(TAG, "release task semaphore");
    }
}
