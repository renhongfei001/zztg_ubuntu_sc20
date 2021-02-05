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
import android.telephony.SubscriptionManager;
import android.util.Slog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

/**
 * Helper for interaction with ImsConfig
 * @hide
 */
public class ImsConfigManager {

    public static boolean DEBUG = true;
    private static final String TAG = ImsConfigManager.class.getSimpleName();

    private final Context mContext;

    public ImsConfigManager(Context context) {
        mContext = context;
    }

    private ImsConfig getImsConfig() {
        ImsConfig imsConfig = null;
        try {
            ImsManager imsManager = ImsManager.getInstance(mContext,
                    SubscriptionManager.getDefaultVoicePhoneId());
            imsConfig = imsManager.getConfigInterface();
        } catch (ImsException e) {
            e.printStackTrace();
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR);
        }

        if (imsConfig == null) {
            PalConstants.throwErrStatus(PalConstants.
                    RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
        }
        return imsConfig;
    }

    public void setProvisionedStringValue(int item, String value) {
        if (DEBUG) Slog.d(TAG, "setProvisionedStringValue, item = " + item + ", value = " + value);
        int status;
        try {
            status = imsStatus2PalStatus(getImsConfig().setProvisionedStringValue(item, value));
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstants.RESULT_ERROR_INVALID_STATE;
        }
        PalConstants.throwErrStatus(status);
    }

    public String getProvisionedStringValue(int item) {
        String result = null;
        int status = PalConstants.RESULT_SUCCESS;
        try {
            result = getImsConfig().getProvisionedStringValue(item);
            if (DEBUG) Slog.d(TAG, "getProvisionedStringValue, item = " + item + ", result = " + result);
            if (result == null || result.length() == 0) {
                status = PalConstants.RESULT_ERROR_INVALID_STATE;
            }
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstants.RESULT_ERROR_INVALID_STATE;
        }

        PalConstants.throwErrStatus(status);
        return result;
    }

    public void setProvisionedIntValue(int item, int value) {
        if (DEBUG) Slog.d(TAG, "setProvisionedIntValue, item = " + item + ", value = " + value);
        int status;
        try {
            status = imsStatus2PalStatus(getImsConfig().setProvisionedValue(item, value));
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstants.RESULT_ERROR_INVALID_STATE;
        }
        PalConstants.throwErrStatus(status);
    }

    public int getProvisionedIntValue(int item) {
        if (DEBUG) Slog.d(TAG, "getProvisionedIntValue, item = " + item);
        int result = -1;
        int status = PalConstants.RESULT_SUCCESS;

        try {
            result = getImsConfig().getProvisionedValue(item);
            if (DEBUG) Slog.d(TAG, "getProvisionedStringValue, item = " + item + ", result = " + result);
        } catch (ImsException e) {
            e.printStackTrace();
            status = PalConstants.RESULT_ERROR_INVALID_STATE;
        }

        PalConstants.throwErrStatus(status);
        return result;
    }

    private int imsStatus2PalStatus(int imsStatus) {
        switch(imsStatus) {
            case ImsConfig.OperationStatusConstants.FAILED:
                return PalConstants.RESULT_ERROR;
            case ImsConfig.OperationStatusConstants.SUCCESS:
                return PalConstants.RESULT_SUCCESS;
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_DISABLED:
                return PalConstants.RESULT_ERROR_INVALID_STATE;
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_NONE:
            case ImsConfig.OperationStatusConstants.UNSUPPORTED_CAUSE_RAT:
                return PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE;
            default:
                return PalConstants.RESULT_ERROR_UNDEFINED;
        }
    }

    public static void setResultValue(String value) {
        //used in frameworks\base\services\core\java\com\android\server\omadm\AsyncTaskWorker.java
    }

    public static void setErrCode(int err) {
        //used in frameworks\base\services\core\java\com\android\server\omadm\AsyncTaskWorker.java
    }
}
