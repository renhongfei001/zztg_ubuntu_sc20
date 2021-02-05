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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.provider.Telephony;
import android.util.Slog;

import com.android.internal.telephony.TelephonyProperties;
import com.android.server.omadm.PalConstants;

public class Apns {

    private static final String TAG = Apns.class.getSimpleName();

    public static boolean DEBUG = true;

    private static final int APN_CLASS_MIN = 1;
    private static final int APN_CLASS_MAX = 5;

    private static final String CLASS_1_TYPES = "'%ims,ia%'";   //locate the apn accuratly
    private static final String CLASS_2_TYPES = "'fota'";
    private static final String CLASS_3_TYPES = "'%default,dun,supl%'";
    private static final String CLASS_4_TYPES = "'%cbs,mms%'";
    private static final String CLASS_5_TYPES = "'%800%'";

    private static final String[] CLASSES_TYPES = new String[] {
        CLASS_1_TYPES,
        CLASS_2_TYPES,
        CLASS_3_TYPES,
        CLASS_4_TYPES,
        CLASS_5_TYPES
    };

    private static String createSelectionByApnClass(int apnClass) {
        if (apnClass < APN_CLASS_MIN || apnClass > APN_CLASS_MAX) {
            return null;
        }
        /* Verizon uses different APNs for different purposes */
        String classTypes = CLASSES_TYPES[apnClass - 1];
        StringBuilder selection = new StringBuilder(Telephony.Carriers.TYPE);
        if (apnClass == 2) //fota
    		selection.append(" = ");
        else
       	    selection.append(" like ");
        selection.append(classTypes);
        if (apnClass < APN_CLASS_MAX) {
            selection.append(" and ");
            selection.append(Telephony.Carriers.NUMERIC);
            selection.append("='");
            selection.append(android.os.SystemProperties.get(
                    TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, ""));
            selection.append("'");
        }
        return selection.toString();
    }

    private static final String[] PROJ_NAMES = new String[]{
        Telephony.Carriers.APN
    };

    public static String getName(Context ctx, int apnClass) {
        int errCode = 0;
        String name = null;
        String selection = createSelectionByApnClass(apnClass);
        if (DEBUG) Slog.d(TAG, "getName.selection [" + apnClass + "] = " + selection);
        if (selection == null) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJ_NAMES,
                selection, null, null);
        if (c != null) {
            if (c.moveToFirst()) {
                name = c.getString(0);
            } else {
                errCode = PalConstants.RESULT_ERROR_INVALID_STATE;
            }
            c.close();
        }
        PalConstants.throwErrStatus(errCode);
        if (DEBUG) Slog.d(TAG, "getName[" + apnClass + "] = " + name);
        return name;
    }

    public static int setName(Context ctx, int apnClass, String apnName) {
        int errCode = 0;
        String name = null;
        ContentValues cv = new ContentValues(1);
        cv.put(Telephony.Carriers.APN, apnName);
        String selection = createSelectionByApnClass(apnClass);
        if (DEBUG) Slog.d(TAG, "setName.selection[" + apnClass + "] = "
        + selection);
        if (selection == null) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        int rows = ctx.getContentResolver().update(
                Telephony.Carriers.CONTENT_URI, cv, selection, null);
        if (DEBUG) Slog.d(TAG, "setName[" + apnClass + "] = " + name +
                ", rows = " + rows);
        if (DEBUG) {
            apnName = getName(ctx, apnClass);
            Slog.d(TAG, "check.setName[" + apnClass + "] = " + apnName);
        }
        if (rows < 1) {
            errCode = PalConstants.RESULT_ERROR_INVALID_STATE;
        }
        PalConstants.throwErrStatus(errCode);
        return rows;
    }

    private static final int IPV4_FLAG = 0x00000001;
    private static final int IPV6_FLAG = 0x00000002;
    private static final int IPV4V6_FLAG = IPV4_FLAG | IPV6_FLAG;

    private static final String[] PROJ_IPVS = new String[]{
        Telephony.Carriers.PROTOCOL
    };

    public static int getIpVersions(Context ctx, int apnClass) {
        int ipVersionMask = 0;
        String ipVString = null;
        int errCode = PalConstants.RESULT_SUCCESS;
        String selection = createSelectionByApnClass(apnClass);
        if (DEBUG) Slog.d(TAG,
                "getIpVersions.selection[" + apnClass + "] = " + selection);
        if (selection == null) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJ_IPVS,
                selection, null, null);
        if (c != null) {
            if (c.moveToFirst()) {
                ipVString = c.getString(0);
            } else {
                errCode = PalConstants.RESULT_ERROR_INVALID_ARGS;
            }
            c.close();
        }
        if (ipVString != null) {
            if (DEBUG) Slog.d(TAG,
                    "IPVersions[" + apnClass + "].string = " + ipVString);
            if (ipVString.equalsIgnoreCase("IP")) {
                 // The Settings app writes "IP" instead of the "IPV4" value
                ipVersionMask |= IPV4_FLAG;
            } else {
                if (ipVString.contains("V4")) {
                    ipVersionMask |= IPV4_FLAG;
                }
                if (ipVString.contains("V6")) {
                    ipVersionMask |= IPV6_FLAG;
                }
            }
        } else if (DEBUG) {
            Slog.d(TAG, "IPVersions[" + apnClass + "] = null");
            errCode = PalConstants.RESULT_ERROR_INVALID_STATE;
        }
        PalConstants.throwErrStatus(errCode);
        return ipVersionMask;
    }

    public static int setIpVersions(Context ctx, int apnClass, int ipvFlags) {
        int errCode = 0;
        int ipVersionMask = 0;
        String selection = createSelectionByApnClass(apnClass);
        if (selection == null) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        if (DEBUG) Slog.d(TAG, "getIpVersions.selection[" + apnClass + "] = "
        + selection);
        String ipvXString = null;
        switch (ipvFlags) {
            case IPV4_FLAG:
                ipvXString = "IP";
                break;

            case IPV6_FLAG:
                ipvXString = "IPV6";
                break;

            case IPV4V6_FLAG:
                ipvXString = "IPV4V6";
                break;

            default:
                break;
        }
        if (ipvXString != null) {
            ContentValues cv = new ContentValues(1);
            cv.put(Telephony.Carriers.PROTOCOL, ipvXString);
            int numRows = ctx.getContentResolver().update(
                    Telephony.Carriers.CONTENT_URI, cv, selection, null);
            if (numRows < 1){
                errCode = PalConstants.RESULT_ERROR_INVALID_STATE;
            }
        }
        PalConstants.throwErrStatus(errCode);
        return ipVersionMask;
    }

    private static final String[] PROJ_IDS = new String[]{
        Telephony.Carriers._ID
    };

    public static int getId(Context ctx, int apnClass) {
        if (apnClass < APN_CLASS_MIN || apnClass > APN_CLASS_MAX) {
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        if (APN_CLASS_MAX == apnClass) {
            return apnClass + 1;
        }
        return apnClass;
    }

    private static final String[] PROJ_ENABLED = new String[]{
        Telephony.Carriers.CARRIER_ENABLED
    };

    public static int isEnabled(Context ctx, int apnCls) {
        int errCode = PalConstants.RESULT_SUCCESS;
        int isEnabledValue = -1;
        String selection = createSelectionByApnClass(apnCls);
        if (DEBUG) Slog.d(TAG, "isEnabled.selection[" + apnCls + "] = "
        + selection);
        if (selection == null) {
            errCode = PalConstants.RESULT_ERROR_INVALID_ARGS;
        }
        Cursor c = ctx.getContentResolver().query(
                Telephony.Carriers.CONTENT_URI, PROJ_ENABLED,
                selection, null, null);
        if (c != null) {
            if (c.moveToFirst()) {
                isEnabledValue = c.getInt(0);
            } else {
                errCode = PalConstants.RESULT_ERROR_INVALID_ARGS;
            }
            c.close();
        }
        PalConstants.throwErrStatus(errCode);
        return isEnabledValue;
    }

    public static int enable(Context ctx, int apnCls, int enable) {
        int errCode = PalConstants.RESULT_SUCCESS;
        String selection = createSelectionByApnClass(apnCls);
        if (DEBUG) Slog.d(TAG, "isEnabled.selection[" + apnCls + "] = "
        + selection);
        if (selection == null) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_ARGS);
        }
        ContentValues cv = new ContentValues(1);
        cv.put(Telephony.Carriers.CARRIER_ENABLED, enable);
        int numRows = ctx.getContentResolver().update(
                Telephony.Carriers.CONTENT_URI, cv,
                selection, null);
        if (numRows < 1) {
            errCode = PalConstants.RESULT_ERROR_INVALID_ARGS;
        }
        PalConstants.throwErrStatus(errCode);
        return errCode;
    }

}
