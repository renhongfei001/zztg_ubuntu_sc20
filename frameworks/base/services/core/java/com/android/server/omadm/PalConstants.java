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

import android.telephony.TelephonyManager;
import android.util.Slog;

/**
 * @hide
 */
public class PalConstants {

    public static boolean DEBUG = true;

    public static final String TAG = PalConstants.class.getSimpleName();

    public static final int RESULT_SUCCESS = 0;
    /**
     * Default error
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR = 1;
    /**
     * some arguments are not valid
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_INVALID_ARGS = 7;
    /**
     * returned value is undefined
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_UNDEFINED = 8;
    /**
     * Operation cannot be executed in the current state
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_INVALID_STATE = 10;
    /**
     * Some services or providers are not available
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE = 11;
    /**
     * Client doesn't have permissions for requested operation
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_PERMISSIONS = 12;
    /**
     * Timer timeout occured during operation
     * Keep in sync with "enum result_states" in the "pal.h" file.
     */
    public static final int RESULT_ERROR_TIMEOUT = 13;

    public static final String  CONNMO_PREFS_NAME      = "ConnMoPrefs";
    public static final String  DEFAULT_T_MPSR         = "2";
    public static final String  DEFAULT_TBSR_CDMA      = "2";
    public static final String  DEFAULT_T_1XRTT        = "22";

    public static final int RINGTONE_VOLUME_LEVELS     = 5;
    public static final int NOTIFICATION_VOLUME_LEVELS = 5;
    public static final int ALARM_VOLUME_LEVELS        = 5;
    public static final int MUSIC_VOLUME_LEVELS        = 5;
    public static final int VOLUME_LEVEL_SIZE          = 25;

    public final static String TELEPHONYMANAGER        = "TelephonyManager";
    public final static String SIM_STATE               = "Sim state";
    public final static String MANUFACTURER            = "manufacturer";
    public final static String CDMA_CELL               = "CdmaCell";
    public final static String NETWORK_OPERATOR        = "Operator";
    public final static String NFC_ADAPTER             = "NfcAdapter";
    public final static String BLUETOOTH_ADAPTER       = "BluetoothAdapter";

    public final static String PERMISSION_DENIED       = "Permission denied";
    public final static String OPERATION_NOT_SUPPORTED = "Operation not supported";
    public final static String NOT_READY               = "is not ready";
    public final static String NOT_IDENTIFIED          = "is not identified";
    public final static String VERSION                 = "1.2";
    public final static int    PARENT_DEVICE           = 0;
    public final static int    CURRENT_DEVICE          = 1;
    public final static int    CHILD_DEVICE            = 2;

    public final static String EMPTY_STRING            = "";
    public final static String DEFAULT_CAMPAIN_NUMBER  = "0";
    public final static String MO_WORK_PATH            = "/data/vendor/verizon/dmclient/data";
    public final static String FIRMWARE_VER_PATH       = "/last_update_firmware_version";
    public final static String CONFIGURATION_VER_PATH  = "/last_update_configuration_version";

    public final static String SMART_DEVICE            = "Smart Device";
    public final static String FEATURE_PHONE           = "Smart module";
    public final static String HOST_OPERATION          = "Host device operation";
    public final static String NOT_AVAILABLE           = "n/a";
    public final static String NOT_AVAILABLE_UPPERCASE = "N/A";

    public final static String IMS_DOMAIN              = "vzims.com";

    public final static int    ADMIN_NET_AVAILABLE     = 1;
    public final static int    ADMIN_NET_UNAVAILABLE   = 2;
    public final static int    ADMIN_NET_LOST          = 3;

    public static final String SECURITY_OPTION_SEPARATOR  = "_";
    public static final String NETWORK_SEPARATOR          = "++";
    public static final String NO_NETWORKS_ARE_AVAILABLE  = "N/A,N/A,N/A";
    public static final String FAKE_MAC_ADDRESS           = "02:00:00:00:00:00";

    public static final int PAL_VOICE_TYPE_NONE = 0;
    public static final int PAL_VOICE_TYPE_1XRTT = 1;
    public static final int PAL_VOICE_TYPE_LTE = 2;
    public static final int PAL_VOICE_TYPE_WIFI = 3;

    public static final int PAL_NW_TYPE_1X = 0;
    public static final int PAL_NW_TYPE_3G = 1;
    public static final int PAL_NW_TYPE_4G = 2;
    public static final int PAL_NW_TYPE_UNKNOWN = 3;

    public static final int PAL_CONNECT_TYPE_WIFI = 0;
    public static final int PAL_CONNECT_TYPE_MOBILE = 1;
    public static final int PAL_CONNECT_TYPE_UNKNOWN = 2;

    public static final int PREF_NW_MODE_NONE = -1;
    public static final int PREF_NW_MODE_GLOBAL = 0;
    public static final int PREF_NW_MODE_LTE_CDMA = 1;
    public static final int PREF_NW_MODE_GSM_UMTS = 2;

    public static final int NW_TYPE_UNKNOWN     = -2;
    public static final int NW_TYPE_NONE        = -1;
    public static final int NW_TYPE_1XRTT       =  0;
    public static final int NW_TYPE_1XEVDO_REV0 =  1;
    public static final int NW_TYPE_1XEVDO_REVA =  2;
    public static final int NW_TYPE_EHRPD       =  3;
    public static final int NW_TYPE_GSM         =  4;
    public static final int NW_TYPE_GPRS        =  5;
    public static final int NW_TYPE_EDGE        =  6;
    public static final int NW_TYPE_UMTS        =  7;
    public static final int NW_TYPE_HSPA        =  8;
    public static final int NW_TYPE_HSPA_P      =  9;
    public static final int NW_TYPE_LTE         = 10;

    public static final int SIM_STATE_ABSENT = 0;
    public static final int SIM_STATE_READY = 1;
    public static final int SIM_STATE_NETWORK_LOCKED = 2;
    public static final int SIM_STATE_PIN_REQUIRED = 3;
    public static final int SIM_STATE_PUK_REQUIREDT = 4;
    public static final int SIM_STATE_UNKNOWN = 5;

    /**
     * Converts platform network types to types that are
     * described in requirements for DM Client
     * @param tmNwType NW type which is returned by
     * TelephonyManager.getNetworkType()
     * @return type for DM Client
     */
    public static int tmNwType2palNwType(int tmNwType) {
        if (DEBUG) Slog.d(TAG, "tmNwType2palNwType(" + tmNwType + ")");
        switch(tmNwType) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
                return NW_TYPE_GPRS;
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return NW_TYPE_EDGE;
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                return NW_TYPE_1XRTT;
            case TelephonyManager.NETWORK_TYPE_IDEN:
                Slog.w(TAG, "tmNwType2palNwType: iDEN not supported");
                throwErrStatus(RESULT_ERROR_INVALID_ARGS);
            case TelephonyManager.NETWORK_TYPE_GSM:
                return PalConstants.PAL_NW_TYPE_1X;
            case TelephonyManager.NETWORK_TYPE_UMTS:
                return NW_TYPE_UMTS;
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
                return NW_TYPE_1XEVDO_REV0;
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
                return NW_TYPE_1XEVDO_REVA;
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
                return NW_TYPE_HSPA;
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                Slog.w(TAG, "tmNwType2palNwType: EVDO_B not supported");
                throwErrStatus(RESULT_ERROR_INVALID_ARGS);
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                return NW_TYPE_EHRPD;
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return NW_TYPE_HSPA_P;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return NW_TYPE_LTE;
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
            default:
                Slog.w(TAG, "tmNwType2palNwType: unknown tmNwType = "
                        + tmNwType);
                throwErrStatus(RESULT_ERROR_INVALID_STATE);
        }
        return NW_TYPE_UNKNOWN;
    }

    /**
     * Throws exception if errCode is not RESULT_SUCCESS
     *
     * @param errCode it is sent via binder as exception
     * with message = ("" + errCode). This message can be
     * parsed in the native client.
     */
    public static void throwErrStatus(int errCode) {
        /*
         * The following exceptions can be write to the parcel:
         * SecurityException, BadParcelableException,
         * IllegalArgumentException, NullPointerException,
         * IllegalStateException
         * @see Parcel.writeException(Exception e)
         */
        switch(errCode) {
            default:
            case RESULT_ERROR:
            case RESULT_ERROR_UNDEFINED:
                throw new NullPointerException(Integer.toString(errCode));
            case RESULT_ERROR_INVALID_ARGS:
                throw new IllegalArgumentException(Integer.toString(errCode));
            case RESULT_ERROR_INVALID_STATE:
                throw new IllegalStateException(Integer.toString(errCode));
            case RESULT_SUCCESS:
                /*
                 * If status is success then no need to throw exception
                 */
                break;
        }
    }
}
