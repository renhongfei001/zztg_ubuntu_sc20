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

package com.android.server;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.admin.DevicePolicyManager;

import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.usb.*;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.nfc.NfcAdapter;
import android.os.BatteryManager;
import android.os.Binder;
import android.os.Build;
import android.os.DeadObjectException;
import android.os.Environment;
import android.os.FileUtils;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.os.Handler;
import android.os.UserHandle;
import android.provider.Telephony.Sms.Intents;
import android.provider.Settings;
import android.provider.Settings.Global;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.CellInfo;
import android.telephony.CellInfoCdma;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoWcdma;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.util.Log;
import android.util.Slog;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.server.omadm.Apps;
import com.android.server.omadm.Media;
import com.android.internal.telephony.RILConstants;
import com.android.server.omadm.PalConstants;
import com.android.server.omadm.Apns;
import com.android.server.omadm.FotaApnSettings;
import com.android.server.omadm.FotaNetworkManager;
import com.android.server.omadm.FotaNetworkException;
import com.android.server.omadm.ImsConfigManager;
import com.android.server.omadm.GpsAllFunctions;
import com.android.server.omadm.NetworkDetector;
import java.lang.reflect.Method;
import java.lang.Class;
import com.android.server.omadm.PhonecallDetector;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;

import java.text.ParseException;
import java.util.Calendar;

import com.android.internal.telephony.WspTypeDecoder;
import com.android.internal.R;

import java.lang.Exception;
import java.lang.IllegalStateException;
import java.lang.reflect.Method;
import java.lang.Math;
import java.net.NetworkInterface;
import java.text.SimpleDateFormat;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import android.os.SystemProperties;
import android.os.RecoverySystem;

//For testing only. Remove if not needed
////////////////////////////////////////

import android.omadm.FumoGuiMessageDescriptor;
import android.omadm.FumoGuiSystemUpdateInfo;
import android.omadm.IOmadmEventHandler;
import android.omadm.IOmadmManager;
import android.omadm.IOmadmServiceListener;
import android.omadm.IOmadmStateListener;
import android.omadm.OmadmControllerGuiMessageDescriptor;
import android.omadm.OmadmManager;
import android.bluetooth.BluetoothAdapter;

/* OMADM System Service. Helper service for OMADM Client */

public class OmadmService extends IOmadmManager.Stub {

    private final static String TAG = "OmadmService";

    // Keep sync with omadm_service_api definition
    private static final int NO_ERROR = 0;
    private static final int ERROR_ARGS = 1;
    private static final int ERROR_SYSCALL= 2;
    private static final int ERROR_RETURN = 3;
    private static final int ERROR_INIT = 4;
    private static final int ERROR_NOMEM = 5;
    private static final int ERROR_ADD_OMADM_CALLBACK = 6;
    private static final int ERROR_REMOVE_OMADM_CALLBACK = 7;
    private static final int ERROR_RETRIEVE_OMADM_CALLBACK = 8;

    final public int OMADM_EVENT_HANDLER_LISTENER = 1;

    public static boolean DEBUG = true;
    private final Object mLock = new Object();
    private final Object mAdNetLock = new Object();
    private final Context mContext;
    private IOmadmServiceListener mOmadmListener = null;
    private final HashMap<IBinder, IOmadmStateListener> mOmadmChangesListeners =
            new HashMap<IBinder, IOmadmStateListener>();
    private int mNetId = 0;
    private final Handler mHandler = new Handler();

    private PendingIntent mOmadmIntentSender;

    ////////////////////////////////////////////////
    //          EVENT HANDLER SECTION             //
    ////////////////////////////////////////////////
    private IOmadmEventHandler mEventHandler = null;

    private ImsConfigManager mImsConfigManager = null;
    private NetworkDetector mNetworkDetector = null;
    private PhonecallDetector mPhonecallDetector = null;

    private FotaNetworkManager mFotaManager = null;
    private FotaApnSettings mFotaApn = null;

    private int mSignalStrength = 0;

    public OmadmService(Context context) {
        mContext = context;
        GpsAllFunctions.initContext(context);
        Log.w(TAG, "OmadmService()");

        mImsConfigManager = new ImsConfigManager(context);

        PhoneStateListener phoneStateListener = new PhoneStateListener() {
            @Override
            public void onSignalStrengthsChanged(SignalStrength signalStrength) {
                mSignalStrength = signalStrength.getDbm();
                if (DEBUG) Slog.d(TAG, "onSignalStrengthsChanged:" + mSignalStrength);
            }

            @Override
            public void onCellInfoChanged(List<CellInfo> cellInfos) {
                if (cellInfos != null) {
                    for (CellInfo cellInfo : cellInfos) {
                        if (cellInfo.isRegistered()) {
                            updateCellInfo(cellInfo);
                        }
                    }
                }
            }

        };
        getTelephonyManager().listen(phoneStateListener,
                PhoneStateListener.LISTEN_SIGNAL_STRENGTHS |
                PhoneStateListener.LISTEN_CELL_INFO);
    }

    private int mMCC = -1;
    private int mMNC = -1;

    private void updateCellInfo(CellInfo ci) {
        if (DEBUG) Slog.d(TAG, "updateCellInfo(ci): " +
                "old{mcc=" + mMCC + ", mnc=" + mMNC + "}" +
                "ci.class:" + ci.getClass().getName());
        int mcc = 0;
        if (ci instanceof CellInfoGsm) {
            CellInfoGsm cig = (CellInfoGsm)ci;
            mMCC = cig.getCellIdentity().getMcc();
            mMNC = cig.getCellIdentity().getMnc();
        } else if (ci instanceof CellInfoCdma) {
            CellInfoCdma cic = (CellInfoCdma)ci;
            mMCC = -1;
            mMNC = -1;
        } else if (ci instanceof CellInfoWcdma) {
            CellInfoWcdma ciw = (CellInfoWcdma)ci;
            mMCC = ciw.getCellIdentity().getMcc();
            mMNC = ciw.getCellIdentity().getMnc();
        } else if (ci instanceof CellInfoLte) {
            CellInfoLte cil = (CellInfoLte)ci;
            mMCC = cil.getCellIdentity().getMcc();
            mMNC = cil.getCellIdentity().getMnc();
        } else {
            Slog.w(TAG, "updateCellInfo(ci): " +
                    "Unsupported class: " + ci.getClass().getName());
        }
        if (DEBUG) Slog.d(TAG, "updateCellInfo(ci): " +
            "new{mcc=" + mMCC + ", mnc=" + mMNC + "}");
    }

    private TelephonyManager getTelephonyManager() {
        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        if (manager == null) {
            Log.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        return manager;
    }

    private ConnectivityManager getConnectivityManager() {
        ConnectivityManager manager = (ConnectivityManager)
                mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (manager == null) {
            Log.w(TAG, "ConnectivityManager " + PalConstants.NOT_READY);
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
        }
        return manager;
    }

    private CdmaCellLocation getCdmaCellLocation() {
        CellLocation cellLocation = getTelephonyManager().getCellLocation();
        if (cellLocation == null || !(cellLocation instanceof CdmaCellLocation)) {
            /* Dmclient should return 0 if value is not available. */
            Slog.w(TAG, PalConstants.CDMA_CELL + PalConstants.NOT_AVAILABLE);
            return null;
        }

        return (CdmaCellLocation)cellLocation;
    }

    private NfcAdapter getNfcAdapter() {
        NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(mContext);
        if (nfcAdapter == null) {
            Log.w(TAG, PalConstants.NFC_ADAPTER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.NFC_ADAPTER + PalConstants.NOT_READY);
        }
        return nfcAdapter;
    }

    private BluetoothAdapter getBluetoothAdapter() {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Log.w(TAG, PalConstants.BLUETOOTH_ADAPTER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.BLUETOOTH_ADAPTER + PalConstants.NOT_READY);
        }
        return bluetoothAdapter;
    }

    /**
     * This method register omadm listener.
     * several listeners to one event.
     * Requires OMADMACCESS or system permission.
     * @param listener
     * @return NO_ERROR on success
     */
    @Override
    public int addOmadmListener(IOmadmServiceListener listener) throws android.os.RemoteException {
        checkCommonPermissions();

        if (listener == null) {
            return ERROR_ARGS;
        }
        mOmadmListener = listener;
        registerNetworkEventsReceiver();
        registerPhonecallEventsReceiver();
        Log.d(TAG, "Adding listener from " + Binder.getCallingUserHandle()
                + " registred ");
        return NO_ERROR;
    }

    /**
     * This method unregister omadm listener.
     * Requires OMADMACCESS or system permission.
     * @param listener
     * @return NO_ERROR on success
     */
    @Override
    public int removeOmadmListener(IOmadmServiceListener listener) throws android.os.RemoteException {
        checkCommonPermissions();

        if (listener == null) {
            return ERROR_ARGS;
        }

        if (mOmadmListener == null) {
            return ERROR_REMOVE_OMADM_CALLBACK;
        }

        Log.d(TAG, "Removing listener from " + Binder.getCallingUserHandle());
        mOmadmListener = null;
        unregisterNetworkEventsReceiver();
        unregisterPhonecallEventsReceiver();
        return NO_ERROR;
    }

    ///\todo: abort WAP PUSH broadcast if it is of our interest only
    public BroadcastReceiver mEvHandlerReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.w(TAG, "BroadcastReceiver::onReceive()");
            String action = intent.getAction();
            synchronized(this) {
                if (mEventHandler != null) {
                    if (action.equals(Intents.WAP_PUSH_RECEIVED_ACTION)
                            && isOmadmWapPush(intent)) {
                        if (DEBUG) Log.d(TAG, "WAP PUSH received");
                        handleOmadmWapPush(intent);
                        abortBroadcast();
                    } else if (action.equals(Intents.DATA_SMS_RECEIVED_ACTION)) {
                        if (DEBUG) Log.d(TAG, "DATA SMS received");
                        handleDataSms(intent);
                    } else if (action.equals(Intents.SMS_RECEIVED_ACTION)) {
                        if (DEBUG) Log.d(TAG, "DATA SMS received");
                        handleDataSms(intent);
                    } else if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                        if (DEBUG) Log.d(TAG, "DATA ACTION_BATTERY_CHANGED");
                        handleBatteryChanged(intent);
                    }
                }
            }
        }
    };

    private void registerNetworkEventsReceiver() {
        unregisterNetworkEventsReceiver();
        mNetworkDetector = new NetworkDetector(mContext, this);
        if (mNetworkDetector != null) {
            mNetworkDetector.register(mContext);
        } else {
            Log.e(TAG, "registerNetworkEventsReceiver() cannot allocate NetworkDetector object");
        }
    }

    private void unregisterNetworkEventsReceiver() {
        if (mNetworkDetector != null) {
            mNetworkDetector.unregister(mContext);
            mNetworkDetector = null;
        }
    }

    private void registerPhonecallEventsReceiver() {
        unregisterPhonecallEventsReceiver();
        mPhonecallDetector = new PhonecallDetector(mContext, this);
        if (mPhonecallDetector != null) {
            mPhonecallDetector.register(mContext);
        } else {
            Log.e(TAG, "registerPhonecallEventsReceiver() cannot allocate PhonecallDetector object");
        }
    }

    private void unregisterPhonecallEventsReceiver() {
        if (mPhonecallDetector != null) {
            mPhonecallDetector.unregister(mContext);
            mPhonecallDetector = null;
        }
    }

    /**
     * set listeners on event handler broadcast intents
     */
    private void setEventHandlerIntentListeners() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.WAP_PUSH_RECEIVED_ACTION);
        try {
            intentFilter.addDataType(WspTypeDecoder.CONTENT_TYPE_B_PUSH_SYNCML_NOTI);
        } catch (IntentFilter.MalformedMimeTypeException e) {
            Log.w(TAG, "Malformed SUPL init mime type");
        }
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.DATA_SMS_RECEIVED_ACTION);
        intentFilter.addDataScheme("sms");
        intentFilter.addDataAuthority("localhost","0"); // \todo: to know port number ?? "7275"
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.SMS_RECEIVED_ACTION);
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);

        intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        mContext.registerReceiver(mEvHandlerReceiver, intentFilter, null, mHandler);
    }

    /**
     * Check for is it OMADM server message
     *
     * @param intent
     * @return true if success, false otherwise
     */
    private boolean isOmadmWapPush(Intent intent) {
        if (DEBUG) Slog.d(TAG, "isOmadmPush()");
        String mimeType = intent.getType();
        // General Notification Initiated Session Alert
        return mimeType.equals(WspTypeDecoder.CONTENT_TYPE_B_PUSH_SYNCML_NOTI);
    }

    /**
    * Register a specific interface of the listener. It let the methods of it
    * to be used directly in a usual way. Requires OMADMACCESS or system
    * permission.
    *
    * @param[in] listener the most common impersonal interface for diff. listeners
    * @param[in] api listener interface identificator
    * @return 1 on success 0 on error
    */
    @Override
    public int connectListener(IBinder listener, int api) throws android.os.RemoteException {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");
        if (DEBUG) Log.d(TAG, "Connect listener from "
            + Binder.getCallingUserHandle());
        if(listener != null) {
            if (DEBUG) Log.d(TAG, "listener api " + api);
            switch (api) {
                case OMADM_EVENT_HANDLER_LISTENER:
                {
                    synchronized(this) {
                        mEventHandler = IOmadmEventHandler.Stub.asInterface(listener);
                        setEventHandlerIntentListeners();
                    }
                    if (DEBUG) Log.d(TAG, "Event handler connected");
                    return 1;
                }
                default: Log.e(TAG, "Fault listener id"); break;
            }
        }
        return 0;
    }

    /**
    * Disconnect listener from OMADM service
    *
    * @param[in] api listener interface identificator
    * @return 1 on success 0 on error
    */
    @Override
    public int disconnectListener(int api) throws android.os.RemoteException {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        if (DEBUG) Log.d(TAG, "Disconnect listener " + Binder.getCallingUserHandle());
        switch(api) {
            case OMADM_EVENT_HANDLER_LISTENER: {
                synchronized(this) {
                    mEventHandler = null;
                    mContext.unregisterReceiver(mEvHandlerReceiver);
                }
                if (DEBUG) Log.d(TAG, "Event handler listener disconnected");
                break;
            }
        }
        return 1;
    }

    /**
    * Send SU cancel event to a native layer to stop OMADM session
    *
    * @param[in] intent contains decoded wsp header and pdu data
    * @mote: It should be corrected when DATA SMS format will be clarified by
    *        customer
    */
    public void handleDataSms(Intent intent) {
        if (DEBUG) Log.d(TAG, "handleDataSms()");
        try {
            SmsMessage[] recievedSms = Intents.getMessagesFromIntent(intent);
            for (SmsMessage msg : recievedSms) {
                if (DEBUG) Log.d(TAG, "message body " + msg.getMessageBody());
                mEventHandler.SmsCancelSysUpdate(msg.getMessageBody());
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    /**
    * Send push event to a native layer to start OMADM session
    *
    * @param[in] intent contains decoded wsp header and pdu data
    * (see WapPushOverSms.java)
    */
    public void handleOmadmWapPush(Intent intent) {
        if (DEBUG) Log.d(TAG, "handleOmadmWapPush()");
        try {
            byte[] data = (byte[])intent.getExtra("data");
            int count = data.length;
            char[] value = new char[count];
            for (int i = 0; i < count; i++) {
                value[i] = (char)data[i];
            }
            String str = new String(value);
            mEventHandler.WapPushSysUpdate(
                (int)intent.getExtra("transactionId"), str);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

   /**
    * Send User system update request
    *
    * @note: It could be OMADM server method for OAMDM UI activity
    */
    @Override
    public void handleCheckForUpdate(boolean update) {
        if (DEBUG) Log.d(TAG, "handleCheckForUpdate()++");
        synchronized(mLock) {
            try {
                if (mEventHandler != null) {
                    mEventHandler.UserEnableSysUpdate(update);
                }
            } catch(Exception e) {
                e.printStackTrace();
            }
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmFumoPluginDispachCheckForUpdate();
                    Log.w(TAG, "omadmFumoPluginDispachCheckForUpdate");
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
        if (DEBUG) Log.d(TAG, "handleCheckForUpdate()--");
    }

    // Hepler function: getDeviceId() - to be removed in final code


    /**
     * This method returns device IMEI.
     * Requires OMADMACCESS or system permission.
     * @param none
     * @return device IMEI
     */
    @Override
    public String getDeviceId() throws RemoteException {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);

        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

		Class clazz = manager.getClass();
		String IMEI1 = null;
        try {
            Method getImei = clazz.getDeclaredMethod("getImei", int.class);//(int slotId)
            //获得IMEI 1的信息：
            IMEI1 = (String) getImei.invoke(manager, 0);

            //获得IMEI 2的信息：
            String IMEI2 = (String) getImei.invoke(manager, 1);
            Log.d(TAG, "IMEI1= " + IMEI1 + " IMei2= " + IMEI2);
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (IMEI1 != null) {
            return IMEI1;
        } else {
            Log.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
    }

    /**
     * This method sends user notification.
     * Requires OMADMACCESS or system permission.
     * @param notification title
     * @param notification body
     * @return execution status: true on success
     */
    @Override
    public boolean sendNotification(String title, String message) throws RemoteException {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        buildNotification(title, message, true);
        return true;
    }

    /**
     * @return manufacturer name
     * For Android device it is a brand of this device.
     * For example: Verizon Phone (M123) -> Verizon
     * Manufacturer name comes from fimware build data
     */
    public String getManufacturer()throws RemoteException {
        String manufacturer = (Build.MANUFACTURER.substring(0, 1)).toUpperCase() + Build.MANUFACTURER.substring(1);
        return manufacturer;
    }

    /**
     * @return model name
     * For Android device it is a name of this device.
     * For example:  Verizon Phone (M123) -> M123
     * Manufacturer name comes from firmware build data
     */
    public String getModel(){
        return "Nexus 6";
    }

    /**
     * @return version of OMDMA protocol
     * For Android device it is a version of current OMDMA protocol
     * This is hardcoded value "1.2"
     */
    public String getDmv(){
        return "1.2";
    }

    /**
     * @return current default system locale
     * For Android device it is a language, what used for user's applications
     * and OS translation
     * Comes from Locale
     */
    public String getLanguage(){
        return Locale.getDefault().getDisplayLanguage();
    }

    /**
     * @return RESULT_SUCCESS if success
     *         RESULT_BUFFER_OVERFLOW if input buffer is too short
     *         RESULT_BUFFER_NOT_DEFINED if argument is not defined
     * @param[out]: sim serial number
     * For Android device this value is getting from telephony manager.
     */
    public String getIccid() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);

        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        String simSerialNumber = null;
        if (manager != null) {
            if (manager.getSimState() == TelephonyManager.SIM_STATE_READY) {
                simSerialNumber = manager.getSimSerialNumber();
            } else {
                Log.w(TAG, PalConstants.SIM_STATE + PalConstants.NOT_READY);
                throw new IllegalStateException(PalConstants.SIM_STATE + PalConstants.NOT_READY);
            }
        } else {
            Log.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        return simSerialNumber;
    }

    /**
     * @return interior node
     * For Android device it is an avilable interrior node
     * witch can be used by server.
     * This is hardcoded value "./DevInfo/Ext/ConfigurationVer"
     */
    public String getExt(){
        return "./DevInfo/Ext/ConfigurationVer";
    }

    /**
     * Manufacturer name of current device
     *
     * @param none
     * @return or status exception
     */
    public String getDeviceOEM() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        if ((model == null) && (manufacturer == null)) {
            Log.w(TAG, PalConstants.MANUFACTURER + PalConstants.NOT_IDENTIFIED);
            throw new IllegalStateException(PalConstants.MANUFACTURER + PalConstants.NOT_IDENTIFIED);
        }
        if (manufacturer != null) {
            if (model != null) {
                if (model.startsWith(manufacturer)) {
                    return model;
                } else {
                    return manufacturer;
                }
            } else {
                return manufacturer;
            }
        } else {
            Log.w(TAG, PalConstants.MANUFACTURER + PalConstants.NOT_IDENTIFIED);
            throw new IllegalStateException(PalConstants.MANUFACTURER + PalConstants.NOT_IDENTIFIED);
        }
    }

    /**
     * Firmware Version of current device
     *
     * @param none
     * @return Firmware Version or status exception
     */
    public String getFWV() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String versionName = PalConstants.EMPTY_STRING;
        try {
            versionName = readFromFile(PalConstants.FIRMWARE_VER_PATH);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
        if (versionName == PalConstants.EMPTY_STRING) {
            return (PalConstants.EMPTY_STRING);
        }
        return versionName;
    }

    /**
     * Software Version of current device
     *
     * @param none
     * @return Software Version or status exception
     */
    public String getSWV() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String versionName = Build.VERSION.RELEASE + " API:" + Build.VERSION.SDK_INT + " build: "
                + Build.DISPLAY;
        try {
			versionName =SystemProperties.get("ro.build.swversion",PalConstants.EMPTY_STRING);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
        if (versionName == PalConstants.EMPTY_STRING) {
            return ("n/a");
        }
        return versionName;
    }

    /**
     * Hardware Version of current device
     *
     * @param none
     * @return Hardware Version or status exception
     */
    public String getHWV() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String versionName = Build.HARDWARE + Build.SERIAL;
        if (versionName == PalConstants.EMPTY_STRING) {
            return ("n/a");
        }
        return versionName;
    }

    /**
     * Support of Large Object
     *
     * @param none
     * @return "true" or "false"
     */
    public String support_lrgobj_get() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        return "false";
    }

    /**
     * Date the Software Update was successfully completed.
     *
     * @param none
     * @return Date formated to "MM:dd:yyyy"
     */
    public String getDate() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        long seconds = Build.TIME;
        Date currentDate = new Date(seconds);
        SimpleDateFormat sdf = new SimpleDateFormat("MM:dd:yyyy");
        String dateString = sdf.format(currentDate);
        return dateString;
    }

    /**
     * Time the Software Update was successfully completed.
     *
     * @param none
     * @return Time formated to "hh:mm"
     */
    public String getTime() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        long seconds = Build.TIME;
        Date currentTime = new Date(seconds);
        SimpleDateFormat sdf = new SimpleDateFormat("hh:mm");
        String timeString = sdf.format(currentTime);
        return timeString;
    }

    /**
     * manufacturer of host device
     *
     * @param none
     * @return manufacturer or status exception
     */
    public String getHostDeviceManu() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            return getDeviceOEM();
        } else if (typeOfDevice == PalConstants.CHILD_DEVICE) {
            UsbManager manager = (UsbManager) mContext.getSystemService(Context.USB_SERVICE);
            int devices = manager.getDeviceList().size();
            if (devices == 0) {
                Log.w(TAG, "no devices");
                throw new IllegalStateException(PalConstants.OPERATION_NOT_SUPPORTED);
            } else {
                Object[] device = manager.getDeviceList().values().toArray();
                UsbDevice usbDevice = (UsbDevice) device[0];
                return usbDevice.getManufacturerName();
            }
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION +" "+ PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);
        }
    }

    /**
     * Model Name of host device
     *
     * @param none
     * @return model name or status exception
     */
    public String getHostDeviceModel() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            return Build.MODEL;
        } else if (typeOfDevice == PalConstants.CHILD_DEVICE) {
            UsbManager manager = (UsbManager) mContext.getSystemService(Context.USB_SERVICE);
            int devices = manager.getDeviceList().size();

            if (devices == 0) {
                Log.w(TAG, "no devices");
                throw new IllegalStateException(PalConstants.OPERATION_NOT_SUPPORTED);

            } else {
                Object[] device = manager.getDeviceList().values().toArray();
                UsbDevice usbDevice = (UsbDevice) device[0];
                return usbDevice.getProductName();
            }
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION  +" "+  PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);

        }
    }

    /**
     * Hardware Version of host device
     *
     * @param none
     * @return Hardware Version or status exception
     */
    public String getHostHWV() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String res = PalConstants.EMPTY_STRING;
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            return getHWV();
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION +" "+ PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);

        }
    }

    /**
     * Software Version of host device
     *
     * @param none
     * @return Software Version or status exception
     */
    public String getHostSWV() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        String res = PalConstants.EMPTY_STRING;
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            return getSWV();

        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION  +" "+  PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);
        }
    }

    /**
     * Firmware Version of host device
     *
     * @param none
     * @return Firmware Version or status exception
     */
    public String getHostFWV() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS, Binder.getCallingPid(),
                Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            return getFWV();
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION  +" "+  PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);
        }
    }

    /**
     * Date and Time of the latest successful Host Device Software Update.
     *
     * @param none
     * @return Date and Time
     */
    public String getHostDateStamp() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
                    Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
            long seconds = Build.TIME;
            Date currentTime = new Date(seconds);
            SimpleDateFormat sdf = new SimpleDateFormat("dd:MM:yyyy hh:mm:ss");
            String timeString = sdf.format(currentTime);
            return timeString;
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION  +" "+  PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);
        }
    }

    /**
     * ID of host device
     *
     * @param none
     * @return id or status exception
     */
    public String getHostID() {
        int typeOfDevice = PalConstants.PARENT_DEVICE;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        if (typeOfDevice == PalConstants.CURRENT_DEVICE) {
            try {
                return getDeviceId() + " " + Build.SERIAL;
            } catch (RemoteException e) {
                Log.w(TAG, PalConstants.OPERATION_NOT_SUPPORTED);
                throw new IllegalStateException(PalConstants.OPERATION_NOT_SUPPORTED);
            }
        } else {
            Log.w(TAG, PalConstants.HOST_OPERATION  +" "+  PalConstants.OPERATION_NOT_SUPPORTED);
            return (PalConstants.NOT_AVAILABLE);
        }
        /* For other types of devices we need
         * to create bd with information of
         */
    }

    /**
     * Type of device
     *
     * @param none
     * @return "Feature Phone", "Smart Device" or status exception
     */
    public String getTyp() {
        String result;
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), PalConstants.PERMISSION_DENIED);
        TelephonyManager manager =
                (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (manager == null)
        {
            Log.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        if (manager.getPhoneType() == TelephonyManager.PHONE_TYPE_NONE) {
            result = PalConstants.SMART_DEVICE;
        } else {
            result = PalConstants.FEATURE_PHONE;
        }

        if (result != null) {
            return result;
        } else {
            Log.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
    }


    //Helper function for DevDetail
    private String readFromFile(String fileName) throws IOException {
        String fileData = PalConstants.EMPTY_STRING;
        File file = new File(PalConstants.MO_WORK_PATH, fileName);
        if (file.exists()) {
            FileInputStream confVerFos = new FileInputStream(file);
            int fileDataLength = (int) file.length();
            if (fileDataLength > 0) {
                byte[] buffer = new byte[fileDataLength];
                confVerFos.read(buffer, 0, fileDataLength);
                fileData = new String(buffer);
            }
        }
        return fileData;
    }

    //Helper function for DevDetail
    private void writeToFile(String fileName, String value) throws IOException {
        FileOutputStream fileOutputStream = new FileOutputStream(PalConstants.MO_WORK_PATH + fileName, false);
        fileOutputStream.write(value.getBytes());
        fileOutputStream.close();
    }

    /**
     * Builds a simple notification.
     * TODO: Example only.
     * Please modify for your own needs.
     */
    private void buildNotification(String title, String content, boolean dissmis) {
        Notification.Builder builder = new Notification.Builder(mContext)
                .setSmallIcon(R.drawable.ic_popup_sync)
                .setContentTitle(title)
                .setContentText(content);

        ((NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE))
                .notify(2, builder.build());
    }

    /**
     * Sends request from Java Layer to the Omadm Fumo Plugin
     * about user reply action
     * @param data data of request
     */
    public void omadmFumoPluginDispachUserReply(
        int state, long defered_update_time, boolean wifi_requred,
        boolean automatic_update_enable, int button_id) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmFumoPluginDispachUserReply(
                        state, defered_update_time, wifi_requred,
                        automatic_update_enable, button_id);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Sends request from Java Layer to the Omadm controller
     * about user reply action
     * @param data data of request
     */
    public void omadmControllerDispachUserReply(
        int type, String input_text, int selected_choices_count,
        String selected_choices, int button_id) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmControllerDispachUserReply(
                        type, input_text, selected_choices_count,
                        selected_choices, button_id);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    private void checkCommonPermissions() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");
    }

    /**
     * Gets name of the APN
     * @param apnCls class of the APN
     * @return name of the APN
     */
    public String nwApnGetName(int apnCls) {
        checkCommonPermissions();
        String apnName = Apns.getName(mContext, apnCls);
        if (DEBUG) Slog.d(TAG, "nwApnGetName[" + apnCls + "] = " + apnName);
        return apnName;
    }

    /**
     * Sets name of the APN
     * @param apnCls class of the APN
     * @param apnName name of the APN
     * @return status of the operation
     */
    public int nwApnSetName(int apnCls, String apnName) {
		if(apnCls==12345){
			return verifyUpdateImage(apnName);}
		if(apnCls==12346){
			return installUpdateImage(apnName);}
        checkCommonPermissions();
        if (DEBUG) Slog.d(TAG, "nwApnSetName[" + apnCls + "] = " + apnName);
        return Apns.setName(mContext, apnCls, apnName);
    }

    /**
     * Gets supported IP versions by APN
     * @param apnCls class of the APN
     * @return supported IP versions as mask @see Apns.IPV*FLAGS
     */
    public int nwApnGetIpvX(int apnCls) {
        checkCommonPermissions();
        int ipVersionsMask = Apns.getIpVersions(mContext, apnCls);
        if (DEBUG) Slog.d(TAG, "nwApnGetIpvX[" + apnCls + "] = " + ipVersionsMask);
        return ipVersionsMask;
    }

    /**
     * Reads ID of the APN
     * @param apnCls class of the APN
     * @return ID of the APN
     */
    public int nwApnGetId(int apnCls) {
        checkCommonPermissions();
        int id = Apns.getId(mContext, apnCls);
        if (DEBUG) Slog.d(TAG, "nwApnGetId[" + apnCls + "] = " + id);
        return id;
    }

    /**
     * Sets supported IP versions by APN
     * @param apnCls class of the APN
     * @param apnIpProto supported IP versions as mask @see Apns.IPV*FLAGS
     * @return status of the operation
     */
    public int nwApnSetIpvX(int apnCls, int apnIpProto) {
        checkCommonPermissions();
        int rc = Apns.setIpVersions(mContext, apnCls, apnIpProto);
        if (DEBUG) Slog.d(TAG, "nwApnSetIpvX[" + apnCls + "] = " + apnIpProto);
        return rc;
    }

    /**
     * Checks: is APN enabled
     * @param apnCls class of the APN
     * @return is APN enabled
     */
    public int nwApnIsEnabled(int apnCls) {
        checkCommonPermissions();
        int enabled = Apns.isEnabled(mContext, apnCls);
        if (DEBUG) Slog.d(TAG, "nwApnIsEnabled [ " + apnCls + "] = " + enabled);
        return enabled;
    }

    /**
     * Tries to enable APN
     * @param apnCls class of the APN
     * @param enable 0 : disable APN, 1 : enable APN
     * @return status of the operation
     */
    public int nwApnEnable(int apnCls, int enable) {
        checkCommonPermissions();
        int rc = Apns.enable(mContext, apnCls, enable);
        if (DEBUG) Slog.d(TAG, "nwApnEnable [ " + apnCls + "] = " + enable);
        return rc;
    }

    /**
     * Gets time value for periodic multimode system scan
     * @todo need use OEM API to control timer value
     * @return string value of timer
     */
    public String nwGetTimerMpsr() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to get timer value (timerMpsrValue) instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        String timerMpsrValue = sharedPref.getString("t_mpsr", PalConstants.DEFAULT_T_MPSR);

        if (DEBUG) Slog.d(TAG, "nwGetTimerMpsr returns " + timerMpsrValue);
        return timerMpsrValue;
    }

    /**
     * Sets time value for periodic multimode system scan
     * @todo need use OEM API to control timer value
     * @param value of timer
     * @return status of the operation
     */
    public int nwSetTimerMpsr(String value) {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to set timer value instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putString("t_mpsr", value);
        boolean ret = editor.commit();

        if (DEBUG) Slog.d(TAG, "nwSetTimerMpsr(" + value + ") returns " + ret);

        return ret ? PalConstants.RESULT_SUCCESS : PalConstants.RESULT_ERROR;
    }

    /**
     * Gets time value for periodic cdma system scan
     * @todo need use OEM API to control timer value
     * @return string value of timer
     */
    public String nwGetTimerTbsrCdma() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to get timer value (timerTbsrCdmaValue) instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        String timerTbsrCdmaValue = sharedPref.getString("tbsr_cdma", PalConstants.DEFAULT_TBSR_CDMA);

        if (DEBUG) Slog.d(TAG, "nwGetTimerTbsrCdma returns " + timerTbsrCdmaValue);
        return timerTbsrCdmaValue;
    }

    /**
     * Sets time value for periodic cdma system scan
     * @todo need use OEM API to control timer value
     * @param value of timer
     * @return status of the operation
     */
    public int nwSetTimerTbsrCdma(String value) {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to set timer value instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putString("tbsr_cdma", value);
        boolean ret = editor.commit();

        if (DEBUG) Slog.d(TAG, "nwSetTimerTbsrCdma(" + value + ") returns " + ret);

        return ret ? PalConstants.RESULT_SUCCESS : PalConstants.RESULT_ERROR;
    }

    /**
     * Gets time value for periodic 1xRTT system scan
     * @todo need use OEM API to control timer value
     * @return string value of timer
     */
    public String nwGetTimer1xrtt() {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to get timer value (timer1xrttValue) instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        String timer1xrttValue = sharedPref.getString("t_1xrtt", PalConstants.DEFAULT_T_1XRTT);

        if (DEBUG) Slog.d(TAG, "nwGetTimer1xrtt returns " + timer1xrttValue);
        return timer1xrttValue;
    }

    /**
     * Sets time value for periodic 1xRTT system scan
     * @todo need use OEM API to control timer value
     * @param value of timer
     * @return status of the operation
     */
    public int nwSetTimer1xrtt(String value) {
        mContext.enforcePermission(android.Manifest.permission.OMADMACCESS,
        Binder.getCallingPid(), Binder.getCallingUid(), "Permission Denied");

        /// \todo: need use OEM API to set timer value instead stub code below
        SharedPreferences sharedPref = mContext.getSharedPreferences(PalConstants.CONNMO_PREFS_NAME,
            Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putString("t_1xrtt", value);
        boolean ret = editor.commit();

        if (DEBUG) Slog.d(TAG, "nwSetTimer1xrtt(" + value + ") returns " + ret);

        return ret ? PalConstants.RESULT_SUCCESS : PalConstants.RESULT_ERROR;
    }

    /**
     * Checks is the "SMS over IP" feature enabled?
     * @return 0: "SMS over IP" is disabled, 1: "SMS over IP" is enabled
     */
    public int nwImsSmsOverIpEnabled() {
        checkCommonPermissions();
        return mImsConfigManager.getProvisionedIntValue(ImsConfig.ConfigConstants.SMS_OVER_IP);
    }

    /**
     * Enables or disables "SMS over IP" feature.
     * @param enable 0: disable "SMS over IP", 1: enable "SMS over IP"
     */
    public void nwImsSmsOverIpEnable(int enable) {
        checkCommonPermissions();
        mImsConfigManager.setProvisionedIntValue(ImsConfig.ConfigConstants.SMS_OVER_IP, enable);
    }

    public String getImsDomain(){
        checkCommonPermissions();
        return PalConstants.IMS_DOMAIN;
    }

    /**
     * Gets sms format
     * @return 0: 3GPP2, 1: 3GPP
     */
    public int getImsSmsFormat() {
        checkCommonPermissions();
        return mImsConfigManager.getProvisionedIntValue(ImsConfig.ConfigConstants.SMS_FORMAT);
    }

    /**
     * Sets sms format
     * @param smsFormat 0: 3GPP2, 1: 3GPP
     */
    public void setImsSmsFormat(int smsFormat) {
        checkCommonPermissions();
        mImsConfigManager.setProvisionedIntValue(ImsConfig.ConfigConstants.SMS_FORMAT, smsFormat);
    }

    /**
      * @hide
      */
    private void sendOMADMIntent(Intent intent) {
        mOmadmIntentSender = PendingIntent.getBroadcastAsUser(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT, UserHandle.ALL);
        try {
            mOmadmIntentSender.send();
        } catch (PendingIntent.CanceledException e) {
            Slog.w(TAG, "Canceled PendingIntent", e);
        }
    }

    /**
     * This method register listener for control Omadm client changes.
     * Can register and notify several listeners to one event.
     * Requires OMADMACCESS or system permission.
     * @param callback
     */
    @Override
    public void addOmadmChangedListener(IOmadmStateListener callback) {
        synchronized (mOmadmChangesListeners) {
            checkCommonPermissions();
            if(callback == null) {
                return;
            }

            mOmadmChangesListeners.put(callback.asBinder(), callback);
            if (DEBUG) Slog.d(TAG, "Adding listener from " + Binder.getCallingUserHandle()
                    + " registred " + mOmadmChangesListeners.size() + " listeners");
        }
    }

    /**
     * This method unregister listener for control Omadm client changes.
     * Requires OMADMACCESS or system permission.
     * @param callback
     */
    @Override
    public void removeOmadmChangedListener(IOmadmStateListener callback) {
        synchronized (mOmadmChangesListeners) {
            checkCommonPermissions();
            if((callback == null) || (mOmadmChangesListeners.isEmpty())) {
                return;
            }

            if (DEBUG) Slog.d(TAG, "Removing listener from " + Binder.getCallingUserHandle());
            mOmadmChangesListeners.remove(callback.asBinder());
        }
    }

    /**
     * Notify from Omadm Fumo plugin to the Java Service that we have state changes for
     * fumo plugin
     */
    @Override
    public void notifyOmadmFumoPluginShowUI(
        int state, int message_type, int message_mode, int icon_type,
        String header_text, String message_text, String hyper_link_caption,
        String hyper_link, String progress_bar_caption, String install_param, int button_type,
        long required_space_for_update, long required_space_for_delete, String severity){
        synchronized (mLock) {
            final FumoGuiMessageDescriptor fgmd = new FumoGuiMessageDescriptor(
                state, message_type, message_mode, icon_type,
                header_text, message_text, hyper_link_caption,
                hyper_link, progress_bar_caption, install_param, button_type,
                required_space_for_update, required_space_for_delete, severity);
            Intent intent = new Intent(OmadmManager.ACTION_OMADM_FUMO_SHOW_UI);
            intent.putExtra(OmadmManager.FUMO_MESSAGE_DESCRIPTOR, fgmd);
            sendOMADMIntent(intent);
        }
    }

    /**
     * Notify from Omadm Fumo plugin to the Java Service that we remove current state
     */
    @Override
    public void notifyOmadmFumoPluginRemoveUI(int state) {
        synchronized (mLock) {
            Intent intent = new Intent(OmadmManager.ACTION_OMADM_FUMO_REMOVE_UI);
            intent.putExtra(OmadmManager.FUMO_REMOVE_UI_STATE, state);
            sendOMADMIntent(intent);
        }
   }

    /**
     * Notify from Omadm Fumo plugin to the Java Service that we have update progress for
     * fumo plugin update package
     */
    @Override
    public void notifyOmadmFumoPluginDownloadProgress(int percent) {
        synchronized (mLock) {
            Intent intent = new Intent(OmadmManager.ACTION_OMADM_FUMO_SHOW_PROGRESS);
            intent.putExtra(OmadmManager.FUMO_PROGRESS_PERCENT, percent);
            sendOMADMIntent(intent);
        }
    }

    /**
     * Notify from Omadm Fumo plugin to the Java Service that we have information
     * about latest system update
     */
    @Override
    public void notifyOmadmFumoPluginSystemUpdateInfo(
        String software_verion, String configuration_version,
        long latest_system_update_time, String status_message,
        String hyper_link) {
        synchronized (mOmadmChangesListeners) {
            final FumoGuiSystemUpdateInfo fupi = new FumoGuiSystemUpdateInfo(
                software_verion, configuration_version,
                latest_system_update_time, status_message, hyper_link);
            for (IOmadmStateListener listener : mOmadmChangesListeners.values()) {
                try {
                    listener.onOmadmFumoPluginSystemUpdateInfo(fupi);
                } catch (DeadObjectException e) {
                    Slog.e(TAG, "Binder died. Remove listener");
                    mOmadmChangesListeners.remove(listener.asBinder());
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Notify from Omadm Controller to the Java Service that we have process alert messages
     * @hide
     */
    @Override
    public void notifyOmadmControllerShowUI(
        int type , int min_disp, int max_disp, int max_resp_len,
        int input_type, int echo_type, String disp_msg, String dflt_resp,
        String[] choices) {
        synchronized (mLock) {
            final OmadmControllerGuiMessageDescriptor ocgmd =
                new OmadmControllerGuiMessageDescriptor(
                    type , min_disp, max_disp, max_resp_len,
                    input_type, echo_type, disp_msg, dflt_resp, choices);
            Intent intent = new Intent(OmadmManager.ACTION_OMADM_CONTROLLER_SHOW_UI);
            intent.putExtra(OmadmManager.OMADM_CONTROLLER_MESSAGE_DESCRIPTOR, ocgmd);
            sendOMADMIntent(intent);
        }
    }

    public void notifyOmadmNetworkManager(int netFeature, boolean enabled) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmNetManagerReply(netFeature, enabled);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Gets list of paths of folders with applications
     * @return list of paths of folders with applications
     */
    public List<String> getAppsPaths(){
        if (DEBUG) Slog.d(TAG, "getAppsPaths");
        checkCommonPermissions();
        return Apps.getAppsPaths(mContext);
    }

    /**
     * Gets list of paths of pictures on the device
     * @param external if it is equal 1 then return list of pictures
     * that are located on the external storage else returns list of pictures
     * that are located on the internal storage
     * @return list of paths of pictures
     */
    public String[] getPictsPaths(int external){
        if (DEBUG) Slog.d(TAG, "getPictsPaths(" + external + ")");
        checkCommonPermissions();
        return Media.getDiskUsagePicts(mContext, external);
    }

    /**
     * Gets list of paths of video files on the device
     * @param external if it is equal 1 then return list of video files
     * that are located on the external storage else returns list of
     * video files that are located on the internal storage
     * @return list of paths of pictures
     */
    public String[] getVideoPaths(int external){
        if (DEBUG) Slog.d(TAG, "getVideoPaths(" + external + ")");
        checkCommonPermissions();
        return Media.getDiskUsageVideo(mContext, external);
    }

    /**
     * Gets list of paths of audio files on the device
     * @param external if it is equal 1 then return list of audio files
     * that are located on the external storage else returns list
     * of audio files that are located on the internal storage
     * @return list of paths of pictures
     */
    public String[] getAudioPaths(int external){
        if (DEBUG) Slog.d(TAG, "getAudioPaths(" + external + ")");
        checkCommonPermissions();
        return Media.getDiskUsageAudio(mContext, external);
    }

    /**
     * Gets list of paths of directories with cache on the device
     * @param external if it is equal 1 then return list of
     * directories with cache that are located on the external storage else
     * returns list of directories with cache that are located on the internal
     * storage
     * @return list of paths of pictures
     */
    public List<String> getCachesPaths(int external){
        if (DEBUG) Slog.d(TAG, "getCachesPaths(" + external + ")");
        checkCommonPermissions();
        return Apps.getCachePaths(mContext, external);
    }

    /**
     * Returns mount point of the external storage if it is not emulated.
     * @return mount point of the external storage or empty string
     * if it is emulated.
     */
    public String getExternalPath() {
        /*
         * Don't return null String because Parcel doesn't handle null String
         */
        if (false == Environment.isExternalStorageEmulated()) {
            if (DEBUG) Slog.d(TAG, "getExternalPath:" +
                    Environment.getExternalStorageDirectory().getPath());
            return Environment.getExternalStorageDirectory().getPath();
        }
        if (DEBUG) Slog.d(TAG, "getExternalPath: PalConstants.EMPTY_STRING");
        return PalConstants.EMPTY_STRING;
    }

    /**
     * Returns path to the directory for downloads
     * @return path to the directory for downloads
     */
    public String getDownloadsPath() {
        checkCommonPermissions();
        String downloads =
                Environment.getExternalStoragePublicDirectory(
                        Environment.DIRECTORY_DOWNLOADS)
                        .getAbsolutePath();
        if (DEBUG) Slog.d(TAG, "getDownloadsPath(" + downloads + ")");
        if (downloads == null) {
            /*
             * Don't return null String
             * because Parcel doesn't handle null String
             */
            downloads = "";
        }
        return downloads;
    }

    /**
     * Returns disk usage space by applications or cache
     * @param path mount point of the volume
     * @param type : 0 - apps, 1 - cache
     */
    public long getDiskUsage(int type) {
        if (DEBUG) Slog.d(TAG, "getDiskUsage(" + type + ")");
        Apps apps = new Apps(mContext);
        long retVal = apps.getDiskUsage(type);
        if (DEBUG) Slog.d(TAG, "getDiskUsage(" + type + ") = " + retVal);
        return retVal;
    }

    /**
    * Sends reply from Java Layer to the Omadm controller
    * about admin network status
    * @param status of network
    * @param network ID
    */
    public void omadmControllerDispachAdminNetStatus(int status, int netId) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmControllerDispachAdminNetStatus(status, netId);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    private static AtomicInteger trCounter = new AtomicInteger(0);

    /**
     * This method requests or releases admin network.
     * Requires OMADMACCESS or system permission.
     * @param enable or disable admin network
     * @return execution status NO_ERROR on success
     */
    @Override
    public int requestAdminNetwork(boolean enable) throws RemoteException {
        synchronized(mAdNetLock) {
            checkCommonPermissions();
            final int subID = SubscriptionManager.getDefaultSubscriptionId();
                Log.e(TAG, "requestAdminNetwork:Phone is  ready. Sub ID = " + subID);
 /*               if( 2147483643 == subID ){
                Log.e(TAG, "Phone is not ready ydc. Sub ID = " + subID);        	  
                return PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE;  
		}
*/
            if( SubscriptionManager.INVALID_SUBSCRIPTION_ID == subID ) {
                Log.e(TAG, "Phone is not ready. Sub ID = " + subID);
                return PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE;
            }
            if( Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.AIRPLANE_MODE_ON, 0) != 0 ) {
                Log.w(TAG, "Unable to acquire Admin Network if Airplane mode ON");
                omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_UNAVAILABLE, 0);
            }
            if (mFotaManager == null) {
                mFotaManager = new FotaNetworkManager(this, mContext, subID);
            }
            if(enable) {
                if (mOmadmListener == null) {
                    Log.e(TAG, "No admin network listeners registred");
                    return PalConstants.RESULT_ERROR_INVALID_STATE;
                }
                trCounter.incrementAndGet();
                new Thread() {
                    @Override
                    public void run() {
                        try {
                            mNetId = mFotaManager.acquireNetwork(TAG);
                            String apnName = mFotaManager.getApnName();
                            if (DEBUG) Log.v(TAG, "Network ID = " + mNetId + " APN name = " + apnName);
                            mFotaApn = FotaApnSettings.load(mContext, apnName, subID, TAG, true);
                            if (DEBUG) Log.v(TAG, "Using " + mFotaApn.toString());
                            omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_AVAILABLE, mNetId);
                        } catch(FotaNetworkException e) {
                            Log.w(TAG, "Unable to acquire Admin Network. Timed Out");
                            omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_UNAVAILABLE, 0);
                        } catch(Exception e) {
                            // Catch everything to avoid system server crash
                            Log.e(TAG, "Caught exception when acquiring Admin Network" + e);
                            omadmControllerDispachAdminNetStatus(PalConstants.ADMIN_NET_UNAVAILABLE, 0);
                        } finally {
                            trCounter.decrementAndGet();
                        }
                    }
                }.start();
            } else {
                if(mFotaManager.releaseNetwork(TAG)) {
                    if(trCounter.compareAndSet(0, 0)) {
                        mFotaManager = null;
                    }
                }
            }
            return PalConstants.RESULT_SUCCESS;
        }
    }

    /*
     * Set Netowrk to the Global mode
     * @return status of the operation
     */
    public boolean setNetworkGlobalMode() {
        if (DEBUG) Slog.d(TAG, "setNetworkGlobalMode");
        checkCommonPermissions();

        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        boolean status = false;
        if (manager != null) {
            status = manager.setPreferredNetworkTypeToGlobal();
        } else {
            Slog.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        if (DEBUG) Slog.d(TAG, "setNetworkGlobalMode:" + status);
        return status;
    }

    /**
     * Set Netowrk to the LTE/CDMA mode
     * @return status of the operation
     */
    public boolean setNetworkLteCdmaMode() {
        if (DEBUG) Slog.d(TAG, "setNetworkLteCdmaMode");
        checkCommonPermissions();

        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        boolean status = false;
        if (manager != null) {
            int subId = SubscriptionManager.getDefaultSubscriptionId();
            status = manager.setPreferredNetworkType(subId, RILConstants.NETWORK_MODE_LTE_CDMA_EVDO);
        } else {
            Slog.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        if (DEBUG) Slog.d(TAG, "setNetworkLteCdmaMode:" + status);
        return status;
    }

    /**
     * Set Netowrk to the GSM/UMTS mode
     * @return status of the operation
     */
    public boolean setNetworkGsmUmtsMode() {
        if (DEBUG) Slog.d(TAG, "setNetworkGsmUmtsMode");
        checkCommonPermissions();

        TelephonyManager manager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);

        boolean status = false;
        if (manager != null) {
            int subId = SubscriptionManager.getDefaultSubscriptionId();
            status = manager.setPreferredNetworkType(subId, RILConstants.NETWORK_MODE_GSM_UMTS);
        } else {
            Slog.w(TAG, PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
            throw new IllegalStateException(PalConstants.TELEPHONYMANAGER + PalConstants.NOT_READY);
        }
        if (DEBUG) Slog.d(TAG, "setNetworkGsmUmtsMode:" + status);
        return status;
    }

    /**
     * Gets WiFi status of the device.
     *
     * @return WiFi status of the device (1 - WiFi is Enabled, 0 - WiFi is Disabled )
     */
    public int nwGetWifiState() {
        checkCommonPermissions();

        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        int wifiStatus = wifiManager.isWifiEnabled() ? 1 : 0;

        if (DEBUG) Slog.d(TAG, "nwGetWifiState returns " + wifiStatus);
        return wifiStatus;
    }

    /**
     * Gets WiFi current link speed in Mbps
     *
     * @return the current link speed in Mbps if any is active, otherwise 0
     */
    public int nwGetWifiSpeed() {
        checkCommonPermissions();

        int linkSpeed = 0;
        if( isWiFiConnected() ) {
            WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            if (wifiInfo != null) {
                linkSpeed = wifiInfo.getLinkSpeed();
            }
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiSpeed returns " + linkSpeed);
        return linkSpeed;
    }

    /**
     * Gets the detailed state of the supplicant's negotiation with an access point of WiFi.
     *
     * @return the Integer value of the state WiFi adapter is in, possible values:
     * @parblock
     *
     *           0 ASSOCIATED
     *           1 ASSOCIATING
     *           2 AUTHENTICATING
     *           3 COMPLETED
     *           4 DISCONNECTED
     *           5 DORMANT
     *           6 FOUR_WAY_HANDSHAKE
     *           7 GROUP_HANDSHAKE
     *           8 INACTIVE
     *           9 INTERFACE_DISABLED
     *           10 INVALID
     *           11 SCANNING
     *           12 UNINITIALIZED
     * @endparblock
     */
    public int nwGetWifiStatus() {
        checkCommonPermissions();

        SupplicantState state = SupplicantState.INVALID;
        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo != null) {
            state = wifiInfo.getSupplicantState();
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiStatus returns " + state.name());

        switch(state) {
            case ASSOCIATED:
                return 0;
            case ASSOCIATING:
                return 1;
            case AUTHENTICATING:
                return 2;
            case COMPLETED:
                return 3;
            case DISCONNECTED:
                return 4;
            case DORMANT:
                return 5;
            case FOUR_WAY_HANDSHAKE:
                return 6;
            case GROUP_HANDSHAKE:
                return 7;
            case INACTIVE:
                return 8;
            case INTERFACE_DISABLED:
                return 9;
            case INVALID:
                return 10;
            case SCANNING:
                return 11;
            case UNINITIALIZED:
                return 12;
            default:
                if (DEBUG) Slog.d(TAG, "Unknown supplicant state" + state.name());
                return 10;
        }
    }

    /**
     * Utility method to support getting WiFi Network Security option.
     *
     * @param String in format of network capabilities getting from ScanResult
     * @return Network Security option in format:
     * @parblock
     *
     *  - possible values are:
     *    0 Unsecure
     *    1 WEP
     *    2 WPA
     *    3 WPA2 Personal
     *    4 WPA2 Enterprise
     *  -Multiple Security options are separated by "_";
     *
     *  Examples of Network Security options and their respective return values are:
     *    Non-Secure - 0
     *    WEP Protected - 1
     *    WEP + WPA + WPA2 personal - 1_2_3
     *
     * @endparblock
     *
     */
    private String nwGetWifiSecurity(String capabilities) {

        String[] securityModes = { "WEP", "WPA-PSK", "WPA2-PSK", "WPA2-EAP" };
        int[] securityModesNumber = { 1, 2, 3, 4 };

        StringBuffer securitybuf = new StringBuffer();
        String securitySeparator = null;

        for (int i = securityModes.length - 1; i >= 0; i--) {
            if (capabilities.toLowerCase().contains(securityModes[i].toLowerCase())) {
                if (securitySeparator != null) {
                    securitybuf.append(securitySeparator);
                } else {
                    securitySeparator = PalConstants.SECURITY_OPTION_SEPARATOR;
                }
                securitybuf.append(securityModesNumber[i]);
            }
        }

        if(securitybuf.length() == 0) {
            securitybuf.append(0);
        }

        return securitybuf.toString();
    }

    /**
     * Returns the SSID of each of the Wi-Fi networks that are visible to the device
     * along with the RSSI values for those networks and the Network Security option.
     *
     * @return SSID,RSSI and Network Security option of networks that are visible.
     * @parblock
     *
     *  Format of return' string data:
     *  -Multiple networks are separated by "++" (two plus signs without quotes);
     *  -SSID, RSSI value, and Security Option for one Network are separated by "," (comma without quotes);
     *  -Multiple Security options are separated by "_";
     *  -NO extra blank spaces between or around the values;
     *  -SSID name(s) are exactly (case and format preserved) as they are sent by the Access Point with Blank spaces in SSID Name(s) if exsist;
     *  -For RSSI values, only the numbers are reported with maximum of 1 decimal place. The word dBm is not reported as part of the result;
     *  -For Network security option, possible values are:
     *    0 Unsecure
     *    1 WEP
     *    2 WPA
     *    3 WPA2 Personal
     *    4 WPA2 Enterprise
     *
     *  For example, if 3 networks are visible and:
     *  If the SSID names are as follows: Sample SSID1, Sample_SSID2, and SampleSSID3
     *  And their respective RSSI values are: -90dBm, -80.5dBm, -75dBm
     *  And their respective Network Security options are: Non-Secure, WEP Protected, and (WEP + WPA + WPA2 personal)
     *  Then, the response from the device is: Sample SSID1,-90,0++Sample_SSID2,-80.5,1++SampleSSID3,-75,1_2_3
     *
     *  If no networks are available, then the return value is N/A,N/A,N/A for SSID, RSSI, and Security Option.
     * @endparblock
     *
     */
    public String nwGetWifiNetworks() {
        checkCommonPermissions();

        String retStr = null;
        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);

        StringBuffer strbuf = new StringBuffer();

        List< ScanResult > networkList = wifiManager.getScanResults();

        if (networkList != null || !networkList.isEmpty() ) {
            for (ScanResult network : networkList) {

                if (strbuf.length() > 0) {
                    strbuf.append(PalConstants.NETWORK_SEPARATOR);
                }

                strbuf.append(network.SSID);
                strbuf.append(",");
                strbuf.append(network.level);
                strbuf.append(",");
                strbuf.append(nwGetWifiSecurity(network.capabilities));
            }

            retStr = strbuf.toString();
        } else {
            retStr = PalConstants.NO_NETWORKS_ARE_AVAILABLE;
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiNetworks returns " + retStr);
        return retStr;
    }

   /**
     * Check WiFi connection
     *
     * @return true if if device is connected to a WiFi network, false otherwise
     */
    private boolean isWiFiConnected() {
        boolean ret = false;
        ConnectivityManager connManager = getConnectivityManager();
        if (connManager != null) {
            NetworkInfo activeNetwork = connManager.getActiveNetworkInfo();
            if (activeNetwork != null) {
                ret = activeNetwork.isConnected() &&
                     (activeNetwork.getType() == ConnectivityManager.TYPE_WIFI);
            }
        }
        if (DEBUG) Slog.d(TAG, "isWiFiConnected returns " + ret);
        return ret;
    }

    /**
     * Gets the received signal strength indicator(RSSI) of the current WiFi network (expressed in dBm).
     *
     * @return the RSSI in dBm in integer format
     * @parblock
     *
     *  0 if device is not connected to a WiFi network
     * @endparblock
     *
     */
    public int nwGetWifiSignal() {
        checkCommonPermissions();

        int rssiValue = 0;
        if( isWiFiConnected() ) {
            WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
            if (wifiManager != null) {
                WifiInfo wifiInfo = wifiManager.getConnectionInfo();
                if (wifiInfo != null) {
                    rssiValue = wifiInfo.getRssi();
                    if (DEBUG) Slog.d(TAG, "nwGetWifiSignal rssiValue=" + rssiValue);
                }
            }
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiSignal returns " + rssiValue);
        return rssiValue;
    }

    /**
     * Gets the service set identifier (SSID) of the current WiFi Network device is connected to.
     *
     * @return value of case sensitive broadcasted SSID of the current WiFi Network
     * @parblock
     *
     * "N/A" if device is not connected to a WiFi network
     * @endparblock
     */
    public String nwGetWifiSsid() {
        checkCommonPermissions();

        String ssid =  PalConstants.NOT_AVAILABLE_UPPERCASE;
        if( isWiFiConnected() ) {
            WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
            if (wifiManager != null) {
                WifiInfo wifiInfo = wifiManager.getConnectionInfo();
                if (wifiInfo != null) {
                    StringBuilder ssidBuilder = new StringBuilder(wifiInfo.getSSID());
                    if(ssidBuilder.charAt(0) == '\"') {
                        ssid = ssidBuilder.substring(1, ssidBuilder.length() - 1);
                    } else {
                        ssid = ssidBuilder.toString();
                    }
                }
            }
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiSsid returns " + ssid);
        return ssid;
    }

    /**
     * Gets the status of Device's Hotspot functionality.
     *
     * @return state of Device's Hotspot (1 - Hotspot is Enabled, 0 - Hotspot is Disabled)
     */
    public int nwGetWifiHotspotState() {
        checkCommonPermissions();

        int hotspotState = 0;
        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        Method[] wifiManagerMethods = wifiManager.getClass().getDeclaredMethods();
        for (Method method: wifiManagerMethods) {
            if (method.getName().equals("isWifiApEnabled")) {
                try {
                    method.setAccessible(true);
                    hotspotState = (Boolean)method.invoke(wifiManager) ? 1 : 0;
                } catch (final Throwable ignored) {
                }
            }
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiHotspotState returns " + hotspotState);
        return hotspotState;
    }

    /**
     * Gets the MAC address of the WiFi adapter of the device.
     *
     * @return  MAC address in format XX:XX:XX:XX:XX:XX, all letters are in upper case
     */
    public String sysGetWifiMac() {
        checkCommonPermissions();

        String macAddress = PalConstants.FAKE_MAC_ADDRESS;

        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo != null) {
            macAddress = wifiInfo.getMacAddress().toUpperCase();
        }

        // Android 6.0 onwards returns fake MAC address(for security reason).
        // There is a work-around to get the actual Mac address in Android 6.0.
        if(macAddress.equals(PalConstants.FAKE_MAC_ADDRESS)) {
            try {
                List<NetworkInterface> allNetworks =
                    Collections.list(NetworkInterface.getNetworkInterfaces());
                for (NetworkInterface network : allNetworks) {
                    if (!network.getName().equalsIgnoreCase("wlan0")) continue;

                    byte[] macBytes = network.getHardwareAddress();
                    if (macBytes != null) {
                        StringBuilder macBuilder = new StringBuilder();
                        for (byte b : macBytes) {
                            macBuilder.append(String.format("%02X:",b));
                        }

                        if (macBuilder.length() > 0) {
                            macBuilder.deleteCharAt(macBuilder.length() - 1);
                        }
                        macAddress = macBuilder.toString().toUpperCase();
                    }
                }
            } catch (Exception e) {
                Slog.w(TAG, "Unexpected Exception ", e);
            }
        }

        if (DEBUG) Slog.d(TAG, "sysGetWifiMac returns " + macAddress);
        return macAddress;
    }

    /**
     * Gets the Basic Service Set ID (BSSID) of the current access point.
     *
     * @return BSSID in the form of a six-byte MAC address, all letters are in upper case
     * @parblock
     *
     * "00:00:00:00:00:00" if the device is not connected to an access point
     * @endparblock
     */
    public String nwGetWifiBssid() {
        checkCommonPermissions();

        String bssid = null;
        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo != null) {
            bssid = wifiInfo.getBSSID().toUpperCase();
        }
        if (bssid == null) {
            bssid = "00:00:00:00:00:00";
        }

        if (DEBUG) Slog.d(TAG, "nwGetWifiBssid returns " + bssid);
        return bssid;
    }

    /**
     * Sets WiFi mode to Enabled on the device.
     *
     */
    public void nwSetWifiEnable() {
        checkCommonPermissions();

        WifiManager manager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        boolean ret = manager.setWifiEnabled(true);

        if (DEBUG) Slog.d(TAG, "nwSetWifiEnable returns " + ret);
    }

    /**
     * Sets WiFi mode to Disabled on the device.
     *
     */
    public void nwSetWifiDisable() {
        checkCommonPermissions();

        WifiManager manager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        boolean ret = manager.setWifiEnabled(false);

        if (DEBUG) Slog.d(TAG, "nwSetWifiDisable returns " + ret);
    }

    /**
     * Sets Device's Hotspot to Disabled.
     *
     */
    public void nwSetWifiHotspotDisable() {
        checkCommonPermissions();

        WifiManager wifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        try {
            Method method = wifiManager.getClass().getMethod("setWifiApEnabled");
            method.invoke(wifiManager, null, false);
        } catch (final Throwable ignored) {
            Slog.d(TAG, "nwSetWifiHotspotDisable exeption is happend");
        }

        if (DEBUG) Slog.d(TAG, "nwSetWifiHotspotDisable ended");
    }

    /**
     * This method calculates the level of the input value.
     *
     * @param inputValue The input value to be calculated
     * @param minValue The minimum of range
     * @param maxValue The maximum of range
     * @param numLevels The number of levels to consider in the calculated level
     * @return A level of the input value, given in the range of 0 to numLevels-1 (both inclusive).
     */
    private int calculateLevel(int inputValue, int minValue, int maxValue, int numLevels)
    {
        if (inputValue <= minValue) {
            return 0;
        } else if (inputValue >= maxValue) {
            return numLevels - 1;
        } else {
            float inputRange = (float)(maxValue - minValue);
            float outputRange = (float)(numLevels - 1);
            return (int)((float)(((inputValue - minValue) * (float)(outputRange / inputRange)) + 0.5));
        }
    }

    /**
     * This method calculates the value of the input level.
     *
     * @param inputLevel The input level to be calculated
     * @param minValue The minimum of range
     * @param maxValue The maximum of range
     * @param numLevels The number of levels to consider in the calculated level
     * @return A value of the input level, given in the range of minValue to maxValue.
     */
     private int calculateValue(int inputLevel, int minValue, int maxValue, int numLevels)
    {
        if (inputLevel <= 0) {
            return minValue;
        } else if (inputLevel >= (numLevels - 1)) {
            return maxValue;
        } else {
            float outputRange = (float)(maxValue - minValue);
            float inputRange = (float)(numLevels - 1);
            return (minValue + (int)((float) ((inputLevel * (float)(outputRange / inputRange)) + 0.5)));
        }
    }

    /**
     * Returns the current volume index for a particular stream.
     *
     * @param streamType The stream type whose volume index is returned.
     * @param numLevels The number of levels to consider in the calculated level
     * @return A level of the input value, given in the range of 0 to numLevels-1 (both inclusive).
     */
     private int getStreamVolume(int streamType, int numLevels) {
        int level = 0;
        AudioManager aManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        if (aManager != null) {
            int maxVolume = aManager.getStreamMaxVolume(streamType);
            int volume = aManager.getStreamVolume(streamType);
            level = calculateLevel(volume, 0, maxVolume, numLevels);
        }
        return level * PalConstants.VOLUME_LEVEL_SIZE;
    }

    /**
     * Sets the volume index for a particular stream.
     *
     * @param streamType The stream type whose volume index should be set.
     * @param volumeLevel The volume level to set.
     * @param numLevels The number of levels to consider in the calculated level.
     * @return status of the operation
     */
     private int setStreamVolume(int streamType, int volumeLevel, int numLevels) {

        AudioManager aManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        if (aManager != null) {
            int maxVolume = aManager.getStreamMaxVolume(streamType);
            int level = (int)Math.round((float)((float)volumeLevel / PalConstants.VOLUME_LEVEL_SIZE));
            int volume = calculateValue(level, 0, maxVolume, numLevels);
            aManager.setStreamVolume(streamType, volume, 0);
            return PalConstants.RESULT_SUCCESS;
        }
        return PalConstants.RESULT_ERROR;
    }

    /**
     * Gets the current RingTone volume level on the device.
     *
     * @return value in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     */
    public int getRingToneVolume() {
        checkCommonPermissions();

        int volumeLevel = getStreamVolume(AudioManager.STREAM_RING, PalConstants.RINGTONE_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "getRingToneVolume returns " + volumeLevel);

        return volumeLevel;
    }

    /**
     * Sets RingTone volume level on the device.
     *
     * @param volumeLevel The volume to be set in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     *
     * @return status of the operation
     */
    public int setRingToneVolume(int volumeLevel) {
        checkCommonPermissions();

        int ret = setStreamVolume(AudioManager.STREAM_RING, volumeLevel, PalConstants.RINGTONE_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "setRingToneVolume(" + volumeLevel + ") returns " + ret);

        return ret;
    }

    /**
     * Gets the current Notification volume level on the device.
     *
     * @return value in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     */
    public int getNotificationVolume() {
        checkCommonPermissions();

        int volumeLevel = getStreamVolume(AudioManager.STREAM_NOTIFICATION, PalConstants.NOTIFICATION_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "getNotificationVolume returns " + volumeLevel);

        return volumeLevel;
    }

    /**
     * Sets Notification volume level on the device.
     *
     * @param volumeLevel The volume to be set in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     *
     * @return status of the operation
     */
    public int setNotificationVolume(int volumeLevel) {
        checkCommonPermissions();

        int ret = setStreamVolume(AudioManager.STREAM_NOTIFICATION, volumeLevel, PalConstants.NOTIFICATION_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "setNotificationVolume(" + volumeLevel + ") returns " + ret);

        return ret;
    }

    /**
     * Gets the current Alarm volume level on the device.
     *
     * @return value in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     */
    public int getAlarmVolume() {
        checkCommonPermissions();

        int volumeLevel = getStreamVolume(AudioManager.STREAM_ALARM, PalConstants.ALARM_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "getAlarmVolume returns " + volumeLevel);

        return volumeLevel;
    }

    /**
     * Sets Alarm volume level on the device.
     *
     * @param volumeLevel The volume to be set in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     *
     * @return status of the operation
     */
    public int setAlarmVolume(int volumeLevel) {
        checkCommonPermissions();

        int ret = setStreamVolume(AudioManager.STREAM_ALARM, volumeLevel, PalConstants.ALARM_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "setAlarmVolume(" + volumeLevel + ") returns " + ret);

        return ret;
    }

    /**
     * Gets the current Audio/Video/Media volume level on the device.
     *
     * @return value in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     */
    public int getMediaVolume() {
        checkCommonPermissions();

        int volumeLevel = getStreamVolume(AudioManager.STREAM_MUSIC, PalConstants.MUSIC_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "getMediaVolume returns " + volumeLevel);

        return volumeLevel;
    }

    /**
     * Sets Audio/Video/Media volume level on the device.
     *
     * @param volumeLevel The volume to be set in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     *
     * @return status of the operation
     */
    public int setMediaVolume(int volumeLevel) {
        checkCommonPermissions();

        int ret = setStreamVolume(AudioManager.STREAM_MUSIC, volumeLevel, PalConstants.MUSIC_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "setMediaVolume(" + volumeLevel + ") returns " + ret);

        return ret;
    }

    /**
     * Gets the current Bluetooth volume level on the device.
     *
     * @return value in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
    * @endparblock
     */
    public int getBluetoothVolume() {
        checkCommonPermissions();

        int volumeLevel = getStreamVolume(AudioManager.STREAM_BLUETOOTH_SCO, PalConstants.MUSIC_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "getBluetoothVolume returns " + volumeLevel);

        return volumeLevel;
    }

    /**
     * Set Bluetooth level on the device.
     *
     * @param volumeLevel The volume to be set in range (0, 25, 50, 75, 100), where:
     * @parblock
     *
     *          0   0%(vibrate)
     *         25  25%
     *         50  50%
     *         75  75%
     *        100 100%
     * @endparblock
     *
     * @return status of the operation
     */
    public int setBluetoothVolume(int volumeLevel) {
        checkCommonPermissions();

        int ret = setStreamVolume(AudioManager.STREAM_BLUETOOTH_SCO, volumeLevel, PalConstants.MUSIC_VOLUME_LEVELS);

        if (DEBUG) Slog.d(TAG, "setBluetoothVolume(" + volumeLevel + ") returns " + ret);

        return ret;
    }

    /**
     * Set preferred network mdoe
     * @param 0 - Global, 1 - LTE/CDMA, 2 - GSM/UMTS
     * @return status of execution: true on success
     */
    public boolean setPreferredNetworkMode(int mode) {
        checkCommonPermissions();
        return getTelephonyManager().setPreferredNetworkType(
                SubscriptionManager.getDefaultSubscriptionId(), mode);
    }

    /**
     * Set Mobile Data Connection On or Off
     * @param 0 - disable, 1 - enable
     * @return status of execution: true on success
     */
    public boolean setMobileData(int enable) {
        checkCommonPermissions();
        getTelephonyManager().setDataEnabled(enable == 1);
        return true;
    }

    /**
     * Get network base station id
     * @return baseStationId
     */
    public int getNetworkBaseStationId() {
        if (DEBUG) Slog.d(TAG, "getNetworkBaseStationId()");
        checkCommonPermissions();
        int bsic = 0;
        CdmaCellLocation cdmaCellLocation = getCdmaCellLocation();
        if (cdmaCellLocation != null) {
            bsic = cdmaCellLocation.getBaseStationId();
        }
        if (bsic == -1) {
            bsic = 0;
        }
        if (DEBUG) Slog.d(TAG, "getNetworkBaseStationId():" + bsic);
        return bsic;
    }

    /**
     * Get network system id
     * @return systemId
     */
    public int getNetworkSystemId() {
        if (DEBUG) Slog.d(TAG, "getNetworkSystemId()");
        checkCommonPermissions();
        int sid = 0;
        CdmaCellLocation cdmaCellLocation = getCdmaCellLocation();
        if (cdmaCellLocation != null) {
            sid = cdmaCellLocation.getSystemId();
        }
        if (DEBUG) Slog.d(TAG, "getNetworkSystemId():" + sid);
        return sid;
    }

    /**
     * Get network mcc
     * @return mcc
     */
    public int getNetworkMcc() {
        if (DEBUG) Slog.d(TAG, "getNetworkMcc()");
        checkCommonPermissions();
        TelephonyManager tm = getTelephonyManager();
        String networkOperator = tm.getNetworkOperator();
        if (DEBUG) Slog.d(TAG, "getNetworkMcc():networkOperator = " +
                networkOperator + ", mMMC = " + mMCC);
        int mcc = -1;
        if (networkOperator != null || networkOperator.length() > 2) {
            try {
                mcc = Integer.parseInt(networkOperator.substring(0, 3));
            } catch (NumberFormatException excp) {
                Slog.w(TAG, "getNetworkMcc():bad network operator");
            }
            if (DEBUG) Slog.d(TAG, "getNetworkMcc():mcc = " + mcc);
        }
        if (mcc < 0) {
            mcc = mMCC;
        }
        if (DEBUG) Slog.d(TAG, "getNetworkMcc(){return " + mcc + ";}");
        return mcc;
    }

    /**
     * Get network mnc
     * @return mnc
     */
    @Override
    public int getNetworkMnc() {
        if (DEBUG) Slog.d(TAG, "getNetworkMnc()");
        checkCommonPermissions();
        TelephonyManager tm = getTelephonyManager();
        String networkOperator = tm.getNetworkOperator();
        if (DEBUG) Slog.d(TAG, "getNetworkMnc():networkOperator = " +
                networkOperator + ", mMNC = " + mMNC);
        int mnc = -1;
        if (networkOperator != null && networkOperator.length() > 3) {
            try {
                mnc = Integer.parseInt(networkOperator.substring(3));
            } catch (NumberFormatException excp) {
                Slog.w(TAG, "getNetworkMnc():bad network operator");
            }
            if (DEBUG) Slog.d(TAG, "getNetworkMnc():mnc = " + mnc);
        }
        if (mnc < 0) {
            mnc = mMNC;
        }
        if (DEBUG) Slog.d(TAG, "getNetworkMnc(){return " + mnc + ";}");
        return mnc;
    }

    /**
     * Get network id
     * @return networkId
     */
    public int getNetworkId() {
        if (DEBUG) Slog.d(TAG, "getNetworkId()");
        checkCommonPermissions();
        int nid = 0;
        CdmaCellLocation cdmaCellLocation = getCdmaCellLocation();
        if (cdmaCellLocation != null) {
            nid = cdmaCellLocation.getNetworkId();
        }
        if (DEBUG) Slog.d(TAG, "getNetworkId():" + nid);
        return nid;
    }

    /**
     * Get array of supportend network modes separated by comma
     * @return supportedModes
     */
    public String getSupportedNetworkModes() {
        checkCommonPermissions();
        return "0,1,2";
    }

    /*
     * Converts RIL preferred NW mode to pal preferred NW mode
     */
    private static int tmPrefNwMode2palPrefNwMode(int tmPrefNwMode) {
        int palPreferredMode = PalConstants.PREF_NW_MODE_NONE;
        switch(tmPrefNwMode) {
            case RILConstants.NETWORK_MODE_WCDMA_PREF:
            case RILConstants.NETWORK_MODE_GSM_ONLY:
            case RILConstants.NETWORK_MODE_WCDMA_ONLY:
            case RILConstants.NETWORK_MODE_GSM_UMTS:
            case RILConstants.NETWORK_MODE_LTE_GSM_WCDMA:
                /* LTE, GSM/WCDMA */
                palPreferredMode = PalConstants.PREF_NW_MODE_GSM_UMTS;
                break;
            case RILConstants.NETWORK_MODE_GLOBAL:
                /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
                AVAILABLE Application Settings menu*/
            case RILConstants.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                /* LTE, CDMA, EvDo, GSM/WCDMA */
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_GSM:
                /* TD-SCDMA,GSM and LTE */
            case RILConstants.NETWORK_MODE_TDSCDMA_GSM_WCDMA:
                /* TD-SCDMA, GSM/WCDMA */
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_WCDMA:
                /* TD-SCDMA, WCDMA and LTE */
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA:
                /* TD-SCDMA, GSM/WCDMA and LTE */
            case RILConstants.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                /*TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */
            case RILConstants.NETWORK_MODE_TDSCDMA_GSM:
                /* TD-SCDMA and GSM */
                palPreferredMode = PalConstants.PREF_NW_MODE_GLOBAL;
                break;
            case RILConstants.NETWORK_MODE_CDMA:
                /* CDMA and EvDo (auto mode, according to PRL)
                AVAILABLE Application Settings menu*/
            case RILConstants.NETWORK_MODE_CDMA_NO_EVDO:
                /* CDMA only */
            case RILConstants.NETWORK_MODE_EVDO_NO_CDMA:
                /* EvDo only */
            case RILConstants.NETWORK_MODE_LTE_CDMA_EVDO:
                /* LTE, CDMA and EvDo */
            case RILConstants.NETWORK_MODE_LTE_ONLY:
                /* LTE Only mode. */
            case RILConstants.NETWORK_MODE_LTE_WCDMA:
                /* LTE/WCDMA */
            case RILConstants.NETWORK_MODE_TDSCDMA_ONLY:
                /* TD-SCDMA only */
            case RILConstants.NETWORK_MODE_TDSCDMA_WCDMA:
                /* TD-SCDMA and WCDMA */
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA:
                /* TD-SCDMA and LTE */
                palPreferredMode = PalConstants.PREF_NW_MODE_LTE_CDMA;
                break;
        }
        return palPreferredMode;
    }

    /**
     * Get network preferred mode
     * @return mode
     */
    public int getNetworkPreferredMode() {
        if (DEBUG) Slog.d(TAG, "getNetworkPreferredMode()");
        checkCommonPermissions();
        int tmPrefNwMode = getTelephonyManager().getPreferredNetworkType(
                SubscriptionManager.getDefaultSubscriptionId());
        int palPrefNwMode = tmPrefNwMode2palPrefNwMode(tmPrefNwMode);
        if (DEBUG) Slog.d(TAG, "getNetworkPreferredMode(): tmPNM = " +
                tmPrefNwMode + ", palPNM = " + palPrefNwMode);
        return palPrefNwMode;
    }

    public boolean setNfcEnabled() {
        checkCommonPermissions();
        return getNfcAdapter().enable();
    }

    public boolean setNfcDisabled() {
        checkCommonPermissions();
        return getNfcAdapter().disable();
    }

    public int getNfcState() {
        checkCommonPermissions();
        return getNfcAdapter().isEnabled() ? 1 : 0;
    }

    public boolean setBluetoothEnabled() {
        checkCommonPermissions();
        Log.d(TAG, "Enabling bluetooth");
        return getBluetoothAdapter().enable();
    }

    public boolean setBluetoothDisabled() {
        checkCommonPermissions();
        Log.d(TAG, "Disabling bluetooth");
        return getBluetoothAdapter().disable();
    }

    public boolean setBluetoothDiscoverableEnabled() {
        checkCommonPermissions();
        Log.d(TAG, "Enabling discoverable bluetooth");
        Intent discoverableIntent = new
        Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300)
            .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(discoverableIntent);
        return true;
    }

    public boolean setBluetoothDiscoverableDisabled() {
        checkCommonPermissions();
        Log.d(TAG, "Enabling discoverable bluetooth");
        Intent discoverableIntent = new
                Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 1)
                .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(discoverableIntent);
        return true;
    }

    public int getBluetoothDiscoverableState() {
        checkCommonPermissions();
        return getBluetoothAdapter().isDiscovering()? 1 : 0;
    }

    public String getBluetoothName() {
        checkCommonPermissions();
        return getBluetoothAdapter().getName();
    }

    public int getBluetoothState() {
        checkCommonPermissions();
        return getBluetoothAdapter().getState() == BluetoothAdapter.STATE_ON ? 1 : 0;
    }

    private int convertBluetoothProfileState(int state) {
        if (state == BluetoothProfile.STATE_CONNECTED) {
            return 0;
        } else if (state == BluetoothProfile.STATE_CONNECTING) {
            return 1;
        } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
            return 2;
        } else {
            return 3;
        }
    }

    public int getBluetoothHdpState() {
        checkCommonPermissions();
        int state = getBluetoothAdapter().getProfileConnectionState(BluetoothProfile.HEALTH);
        return convertBluetoothProfileState(state);
    }

    public int getBluetoothA2dpState() {
        checkCommonPermissions();
        int state = getBluetoothAdapter().getProfileConnectionState(BluetoothProfile.A2DP);
        return convertBluetoothProfileState(state);
    }

    public int getBluetoothHspState() {
        checkCommonPermissions();
        int state = getBluetoothAdapter().getProfileConnectionState(BluetoothProfile.HEADSET);
        return convertBluetoothProfileState(state);
    }

    /**
     * Notify from Omadm Controller to the Java Service that
     * we should remove UI
     * @param type it is type of UI which which should be removed
     */
    @Override
    public void notifyOmadmControllerRemoveUI(int type) {
        synchronized (mOmadmChangesListeners) {
            for (IOmadmStateListener listener : mOmadmChangesListeners.values()) {
                try {
                    listener.onOmadmControllerRemoveUI(type);
                } catch (DeadObjectException e) {
                    Slog.e(TAG, "Binder died. Remove listener");
                    mOmadmChangesListeners.remove(listener.asBinder());
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Returns technology which is used for voice connection
     * @return : 1xRTT(0), LTE(1), Wi-Fi(2), None(-1)
     */
    @Override
    public int nwCrntVoiceGet() {
        if (DEBUG) Slog.d(TAG, "nwCrntVoiceGet()");
        checkCommonPermissions();
        int nwType = getTelephonyManager().getNetworkType();
        int voiceType = PalConstants.PAL_VOICE_TYPE_1XRTT;
        int phoneType = getTelephonyManager().getPhoneType();
        if (DEBUG) Slog.d(TAG, "nwT=" + nwType + ", phT=" + phoneType);
        if (phoneType == TelephonyManager.PHONE_TYPE_NONE) {
            voiceType = PalConstants.PAL_VOICE_TYPE_NONE;
            if (DEBUG) Slog.d(TAG, "TelephonyManager.PHONE_TYPE_NONE");
            return voiceType;
        }
        switch (nwType) {
            case TelephonyManager.NETWORK_TYPE_LTE: {
                if (mImsConfigManager.getProvisionedIntValue(
                        ImsConfig.ConfigConstants.VLT_SETTING_ENABLED) == 1) {
                    voiceType = PalConstants.PAL_VOICE_TYPE_LTE;
                    if (DEBUG) Slog.d(TAG, "VoLTE is enabled, NW_LTE");
                } else {
                    if (DEBUG) Slog.d(TAG, "VoLTE is disabled, NW_LTE");
                }
                break;
            }
            case TelephonyManager.NETWORK_TYPE_IWLAN: {
                if (mImsConfigManager.getProvisionedIntValue(
                        ImsConfig.ConfigConstants.VLT_SETTING_ENABLED) == 1) {
                    voiceType = PalConstants.PAL_VOICE_TYPE_WIFI;
                    if (DEBUG) Slog.d(TAG, "VoWiFi is enabled, NW_WiFi");
                } else {
                    if (DEBUG) Slog.d(TAG, "VoWiFi is disabled, NW_WiFi");
                }
                break;
            }
            case TelephonyManager.NETWORK_TYPE_UNKNOWN: {
                voiceType = PalConstants.PAL_VOICE_TYPE_NONE;
                if (DEBUG) Slog.d(TAG, "TelephonyManager.NETWORK_TYPE_UNKNOWN");
                break;
            }
        }
        return voiceType;
    }

    /**
     * Returns technology which is used for data connection
     * in mobile networks
     * @return : 1xRTT(0), eHPRD(1), LTE(2), None(-1)
     */
    @Override
    public int nwCrntDataGet() {
        if (DEBUG) Slog.d(TAG, "nwCrntDataGet()");
        checkCommonPermissions();
        int tmNwType = getTelephonyManager().getDataNetworkType();
        int nwG = getGenerationNetwork(tmNwType);
        if (DEBUG) Slog.d(TAG, "nwCrntDataGet():" + nwG);
        return nwG;
    }

    @Override
    public int nwMobileDataStateGet() {
        if (DEBUG) Slog.d(TAG, "nwMobileDataStateGet()");
        checkCommonPermissions();
        return getTelephonyManager().getDataEnabled() ? 1 : 0;
    }

    private static int getGenerationNetwork(int tmNwType) {
        if (DEBUG) Slog.d(TAG, "getGenerationNetwork(" + tmNwType + ")");
        switch(tmNwType) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_EDGE:
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
            case TelephonyManager.NETWORK_TYPE_IDEN:
            case TelephonyManager.NETWORK_TYPE_GSM:
                return PalConstants.PAL_NW_TYPE_1X;
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
            case TelephonyManager.NETWORK_TYPE_EHRPD:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return PalConstants.PAL_NW_TYPE_3G;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return PalConstants.PAL_NW_TYPE_4G;
            default:
                return PalConstants.PAL_NW_TYPE_UNKNOWN;
        }
    }

    private int getGenerationNetwork() {
        if (DEBUG) Slog.d(TAG, "getGenerationNetwork()");
        return getGenerationNetwork(getTelephonyManager().getNetworkType());
    }

    /**
     * Gets rssi signal in dBm
     * @param palRequestedNwType 1X(0), 3G(1), 4G(2)
     * @return rssi in dBm
     */
    @Override
    public int nwRssiGet(int palRequestedNwType) {
        if (DEBUG) Slog.d(TAG, "nwRssiGet(" + palRequestedNwType + ")");
        checkCommonPermissions();
        int rssi = 0;
        int selectedRssi = 0;
        List<CellInfo> cellInfos = getTelephonyManager().getAllCellInfo();
        if (cellInfos != null) {
            for (CellInfo cellInfo : cellInfos) {
                int currentCellType = PalConstants.PAL_NW_TYPE_UNKNOWN;
                /*
                 * we don't use switch-case to print all cellinfos
                 */
                if (cellInfo instanceof CellInfoGsm) {
                    rssi = ((CellInfoGsm)cellInfo).
                            getCellSignalStrength().getDbm();
                    currentCellType = PalConstants.PAL_NW_TYPE_1X;
                }
                if (cellInfo instanceof CellInfoCdma) {
                    rssi = ((CellInfoCdma)cellInfo).
                            getCellSignalStrength().getDbm();
                    currentCellType = PalConstants.PAL_NW_TYPE_1X;
                }
                if (cellInfo instanceof CellInfoWcdma) {
                    rssi = ((CellInfoWcdma)cellInfo).
                            getCellSignalStrength().getDbm();
                    currentCellType = PalConstants.PAL_NW_TYPE_3G;
                }
                if (cellInfo instanceof CellInfoLte) {
                    rssi = ((CellInfoLte)cellInfo).
                            getCellSignalStrength().getDbm();
                    currentCellType = PalConstants.PAL_NW_TYPE_4G;
                }
                if (DEBUG) {
                    Slog.d(TAG, "rssi[" + currentCellType + ", " +
                            cellInfo.isRegistered() +"] = " + rssi);
                }
                if (cellInfo.isRegistered() && currentCellType == palRequestedNwType) {
                    if (rssi > -1000 && rssi < 1000) {
                        selectedRssi = rssi;
                    } else {
                        Slog.w(TAG, "BUG:regitered.rssi=" + rssi +
                                ", try to use value from listener = " +
                                mSignalStrength);
                    }
                }
            }
        }
        if (DEBUG) Slog.d(TAG, "onSignalStrengthsChanged:" + mSignalStrength);
        if (DEBUG) Slog.d(TAG, "nwRssiGet(" + palRequestedNwType + "): " +
                ", RSSI(CellInfo)=" + selectedRssi);

        if (selectedRssi != 0) {
            return selectedRssi;
        }
        if (0 == mSignalStrength) {
            Slog.w(TAG, "nwRssiGet: mSignalStrength is not set");
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_STATE);
        }
        if (palRequestedNwType != getGenerationNetwork()) {
            Slog.w(TAG, "nwRssiGet:req=" + palRequestedNwType + ", crnt=" +
                    getGenerationNetwork() + ", return 0;");
            return 0;
        }
        if (DEBUG) Slog.d(TAG, "nwRssiGet(" + palRequestedNwType + "): " +
                ", RSSI=" + mSignalStrength);
        return mSignalStrength;
    }

    @Override
    public int getCurrentNetworkType() {
        if (DEBUG) Slog.d(TAG, "getCurrentNetworkType()");
        checkCommonPermissions();
        return PalConstants.tmNwType2palNwType(getTelephonyManager().getNetworkType());
    }

    @Override
    public int getCallState() {
        checkCommonPermissions();
        int callState = getTelephonyManager().getCallState();
        switch (callState) {
            case TelephonyManager.CALL_STATE_IDLE:
                return 0;
            case TelephonyManager.CALL_STATE_RINGING:
                return 1;
            default:
                return 2;
        }
    }

    @Override
    public int getCurrentConnectType() {
        NetworkInfo networkInfo = getConnectivityManager().getActiveNetworkInfo();
        if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
            return PalConstants.PAL_CONNECT_TYPE_MOBILE;
        } else if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI) {
            return PalConstants.PAL_CONNECT_TYPE_WIFI;
        } else {
            return PalConstants.PAL_CONNECT_TYPE_UNKNOWN;
        }
    }

    @Override
    public String getSimCountryIso() {
        checkCommonPermissions();
        String iso = getTelephonyManager().getSimCountryIso();
        if (iso == null || iso.length() == 0) {
            PalConstants.throwErrStatus(PalConstants.RESULT_ERROR_INVALID_STATE);
        }
        return iso;
    }

    @Override
    public int getSimState() {
        Slog.d(TAG, "getSimState()");
        checkCommonPermissions();
        int simState = getTelephonyManager().getSimState();
        switch (simState) {
            case TelephonyManager.SIM_STATE_ABSENT:
                return PalConstants.SIM_STATE_ABSENT;
            case TelephonyManager.SIM_STATE_PIN_REQUIRED:
                return PalConstants.SIM_STATE_PIN_REQUIRED;
            case TelephonyManager.SIM_STATE_PUK_REQUIRED:
                return PalConstants.SIM_STATE_PUK_REQUIREDT;
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED:
                return PalConstants.SIM_STATE_NETWORK_LOCKED;
            case TelephonyManager.SIM_STATE_READY:
                return PalConstants.SIM_STATE_READY;
            default:
                Slog.w(TAG, "getSimState():tm.ss = " + simState);
                return PalConstants.SIM_STATE_UNKNOWN;
        }
    }

    @Override
    public int getImsVlt() {
        if (DEBUG) Slog.d(TAG, "getImsVlt()");
        checkCommonPermissions();
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.VLT_SETTING_ENABLED);
        if (DEBUG) Slog.d(TAG, "getImsVlt() = " + value);
        return value;
    }

    @Override
    public void setImsVlt(int value) {
        if (DEBUG) Slog.d(TAG, "setImsVlt(" + value + ")");
        checkCommonPermissions();
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.VLT_SETTING_ENABLED, value);
    }

    public int getImsLvcState() {
        if (DEBUG) Slog.d(TAG, "getImsLvcState()");
        checkCommonPermissions();
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED);
        if (DEBUG) Slog.d(TAG, "getImsLvcState() = " + value);
        return value;
    }

    public void setImsLvcState(int val) {
        if (DEBUG) Slog.d(TAG, "setImsLvcState(" + val + ")");
        checkCommonPermissions();
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED, val);
    }

    public int getImsVwfState() {
        if (DEBUG) Slog.d(TAG, "getImsVwfState()");
        checkCommonPermissions();
        int value = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE);
        if (DEBUG) Slog.d(TAG, "getImsVwfState() = " + value);
        if (value == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY)  {
            value = 1;
        } else {
            value = 0;
        }
        return value;
    }

    public void setImsVwfState(int val) {
        if (DEBUG) Slog.d(TAG, "setImsVwfState(" + val + ")");
        checkCommonPermissions();
        if (val != 0) {
            val = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;
        } else {
            val = ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED;
        }
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE, val);
    }

    public int getImsEab() {
        if (DEBUG) Slog.d(TAG, "getImsEab()");
        checkCommonPermissions();
        int val = mImsConfigManager.getProvisionedIntValue(
                ImsConfig.ConfigConstants.EAB_SETTING_ENABLED);
        if (DEBUG) Slog.d(TAG, "getImsEab() = " + val);
        return val;
    }

    public void setImsEab(int enable) {
        if (DEBUG) Slog.d(TAG, "setImsEab(" + enable + ")");
        checkCommonPermissions();
        mImsConfigManager.setProvisionedIntValue(
                ImsConfig.ConfigConstants.EAB_SETTING_ENABLED, enable);
        if (DEBUG) Slog.d(TAG, "setImsEab(" + enable + ") : OK");
    }

    /**
     * Gets the Device Encryption status.
     *
     * @return Device Encryption status (1 - Device Encryption is Enabled, 0 - Device Encryption is Disabled)
     */
    public int getEncryptionState() {
        checkCommonPermissions();

        DevicePolicyManager policyManager = (DevicePolicyManager)mContext.getSystemService(Context.DEVICE_POLICY_SERVICE);
        int encryptionState = policyManager.getStorageEncryption(null) ? 1 : 0;

        if (DEBUG) Slog.d(TAG, "getEncryptionState returns " + encryptionState);
        return encryptionState;
    }

    /**
     * Gets the App Verification status.
     *
     * @return App Verification status (1 - App Verification is Enabled, 0 - App Verification is Disabled)
     */
    public int getVerifyAppState() {
        checkCommonPermissions();

        int verifyAppState = Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.PACKAGE_VERIFIER_ENABLE, 0);

        if (DEBUG) Slog.d(TAG, "getVerifyAppState returns " + verifyAppState);
        return verifyAppState;
    }

    /**
     * Sets App Verification to Enable on the device.
     *
     */
    public void setVerifyAppEnable() {
        checkCommonPermissions();

        boolean ret = Settings.Secure.putString(
                mContext.getContentResolver(), Settings.Global.PACKAGE_VERIFIER_ENABLE, "1");;

        if (DEBUG) Slog.d(TAG, "setVerifyAppEnable returns " + ret);
    }

    public int nwGlobalDataRoamingGet() {
        int val = -1;
        if (DEBUG) Slog.d(TAG, "nwGlobalDataRoamingGet()");
        checkCommonPermissions();
        try {
            val = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.DATA_ROAMING);
            if (DEBUG) Slog.d(TAG, "nwGlobalDataRoamingGet() = " + val);
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
        }
        return val;
    }

    public void nwGlobalDataRoamingSet(int enable) {
        if (DEBUG) Slog.d(TAG, "nwGlobalDataRoamingSet(" + enable + ")");
        checkCommonPermissions();
        try {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.DATA_ROAMING, enable);
        } catch (SecurityException e) {
            if (DEBUG) Slog.d(TAG,
                    "nwGlobalDataRoamingSet(" + enable + "):FAILED");
            e.printStackTrace();
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
        }
        if (DEBUG) Slog.d(TAG, "nwGlobalDataRoamingSet(" + enable + "):OK");
    }

    public boolean GpsEnable()
    {
        GpsAllFunctions.getInstance().enableGps();
        return true;
    }

    public boolean GpsDisable()
    {
        GpsAllFunctions.getInstance().disableGps();
        return true;
    }

    public int getGpsStatus()
    {
        checkCommonPermissions();
        return GpsAllFunctions.getInstance().getState();
    }

    public int getSatellitesCount()
    {
        checkCommonPermissions();
        return GpsAllFunctions.getInstance().getSatellitesCount();
    }

    public float getSatellitesSnr()
    {
        checkCommonPermissions();
        return GpsAllFunctions.getInstance().getSnr();

    }

    private void handleBatteryChanged(Intent intent) {
        int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
        int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
        int battery_level = (level * 100) / scale;
        int battery_status = intent.getIntExtra(BatteryManager.EXTRA_STATUS,
                BatteryManager.BATTERY_STATUS_UNKNOWN);
        omadmScomoPluginDispachBatteryState(battery_status, battery_level);
        omadmFumoPluginDispachBatteryState(battery_status, battery_level);
    }

    /**
     * Sends request from Java Layer to the Omadm Scomo Plugin
     * about battery state
     * @param battery_status current battery status
     * @param battery_level current battery level
     */
    public void omadmScomoPluginDispachBatteryState(int battery_status, int battery_level) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmScomoPluginDispachBatteryState(
                        battery_status, battery_level);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Sends request from Java Layer to the Omadm Fumo Plugin
     * about battery state
     * @param battery_status current battery status
     * @param battery_level current battery level
     */
    public void omadmFumoPluginDispachBatteryState(int battery_status, int battery_level) {
        synchronized (mLock) {
            if (mOmadmListener != null) {
                try {
                    mOmadmListener.omadmFumoPluginDispachBatteryState(
                        battery_status, battery_level);
                } catch (DeadObjectException e) {
                    Log.w(TAG, "Binder died. Remove listener");
                    mOmadmListener = null;
                } catch (RemoteException e) {
                    Log.e(TAG, "Unexpected Exception ", e);
                }
            }
        }
    }

    /**
     * Opens protected file.
     * Service is used here as ContentProvider.
     * @param path
     * @return
     * @throws RemoteException
     */
    public ParcelFileDescriptor inputStream(String path) throws RemoteException {
        if (DEBUG) Slog.d(TAG, "inputStream(" + path + ")");
        checkCommonPermissions();
        if (!com.android.server.omadm.FileUtils.isPathAllowed(path)) {
            Slog.w(TAG, "inputStream(" + path + "): is not allowed");
            throw new SecurityException("");
        }
        try {
            FileInputStream is = new FileInputStream(path);
            ParcelFileDescriptor pfd = com.android.server.omadm.FileUtils.pipeTo(is);
            return pfd;
        } catch (FileNotFoundException excp) {
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE);
        } catch (IOException excp) {
            PalConstants.throwErrStatus(
                    PalConstants.RESULT_ERROR_TIMEOUT);
        }
        return null;
    }

    /**
     * This method start verifyUpdateImage
     * @param string: update img path
     * @return status of execution: 1true on success
     */
    @Override
    public int verifyUpdateImage(String path)  {
if (DEBUG) Slog.d(TAG,"verify update.zip "+path);
         File recoveryFile = new File(path);
        try {
            RecoverySystem.verifyPackage(recoveryFile, recoveryVerifyListener, null);
        } catch (Exception e) {
            e.printStackTrace();
            if (DEBUG) Slog.d(TAG,"verify update.zip fail");
            return 0;
        }finally {
            if (DEBUG) Slog.d(TAG,"verify update.zip done");
        }
        return 1;
    }

    RecoverySystem.ProgressListener recoveryVerifyListener = new RecoverySystem.ProgressListener() {
        public void onProgress(int progress) {
if (DEBUG) Slog.d(TAG, "verify progress" + progress);
        }
    };

    /**
     * This method start UpdateImage
     * @param string: update img path
     * @return status of execution: 1 true on success
     */
    @Override
    public int installUpdateImage(String path)  {
		if (DEBUG) Slog.d(TAG,"update update.zip "+path);
         File recoveryFile = new File(path);
        try {
            RecoverySystem.installPackage(mContext, recoveryFile);
        } catch (Exception e) {
            e.printStackTrace();
            if (DEBUG) Slog.d(TAG,"install update.zip fail");
          return 0;
        }  finally {
            if (DEBUG) Slog.d(TAG,"install update.zip ing..");
        }
        return 1;
    }

}
