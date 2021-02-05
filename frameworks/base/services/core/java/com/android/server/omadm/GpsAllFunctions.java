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

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Slog;

import java.lang.Exception;
import java.lang.Runnable;
import java.util.Iterator;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Created by ruykarpunin on 16.06.16.
 */
public class GpsAllFunctions implements LocationListener, GpsStatus.Listener{
    private static Context mContext = null;
    private int satellitesCount =0 ;
    private float snr = 0;
    private Handler mHandler = null;
    private static GpsAllFunctions HOLDER_INSTANCE = null;
    private LocationManager mLocationManager = null;

    private GpsAllFunctions() {}

    public static synchronized GpsAllFunctions getInstance() {
        if (HOLDER_INSTANCE == null) {
            HOLDER_INSTANCE = new GpsAllFunctions();
        }
        return HOLDER_INSTANCE;
    }

    public static void initContext(Context context) {
        mContext = context;
    }

    public float getSnr()
    {
        Slog.d("Gps", "snr: " + String.valueOf(snr));
        return snr;
    }

    public int getSatellitesCount()
    {
        Slog.d("Gps", "Satellites: " + String.valueOf(satellitesCount));
        return satellitesCount;
    }

    public void enableGps() {
        Settings.Secure.setLocationProviderEnabled(mContext.getContentResolver(), LocationManager.GPS_PROVIDER, true);
        Handler handler = new Handler(Looper.getMainLooper());
        mLocationManager = (LocationManager)
                mContext.getSystemService(Context.LOCATION_SERVICE);
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                try {
                    GpsAllFunctions gpsAllFunctions = GpsAllFunctions.getInstance();
                    mLocationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, gpsAllFunctions);
                    mLocationManager.addGpsStatusListener(gpsAllFunctions);
                    Slog.d("Gps", "Listeners were attached");
                } catch (Exception e) {
                    Slog.d("Gps", "Attaching gps listeners " + e.getMessage());
                }
            }
        };
        handler.post(runnable);
    }

    public void disableGps() {
        try {
            mLocationManager.removeGpsStatusListener(this);
            final long identity = Binder.clearCallingIdentity();
            mLocationManager.removeUpdates(this);
            Binder.restoreCallingIdentity(identity);
            Settings.Secure.setLocationProviderEnabled(mContext.getContentResolver(), LocationManager.GPS_PROVIDER, false);
        } catch (Exception e) {
            Slog.d("Gps", "Deattaching gps listeners " + e.getMessage());
        }
    }

    public int getState()
    {
        LocationManager lm = (LocationManager) mContext.getSystemService(mContext.LOCATION_SERVICE);
        boolean gps_on = lm.isProviderEnabled(LocationManager.GPS_PROVIDER);
        return (gps_on) ? 1 : 0;
    }

    @Override
    public void onLocationChanged(Location location) {
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
    }

    @Override
    public void onProviderEnabled(String provider) {
        //TODO Handle onProviderEnabled it if needed.
        Slog.d("Gps", "Provider " + provider + " was enabled");
    }

    @Override
    public void onProviderDisabled(String provider) {
        //TODO Handle onProviderDisabled it if needed.
        Slog.d("Gps", "Provider " + provider + " was disabled");
    }

    @Override
    public void onGpsStatusChanged(int event) {
        switch( event ) {
            case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                try {
                    final GpsStatus gs = mLocationManager.getGpsStatus(null);
                    int i = 0;
                    snr = 0;
                    final Iterator<GpsSatellite> it = gs.getSatellites().iterator();
                    while (it.hasNext()) {
                        GpsSatellite satellite = it.next();
                        snr += satellite.getSnr();
                        i += 1;
                    }
                    satellitesCount = i;
                    snr /= (float) i;
                } catch (Exception e) {
                    Slog.d("Gps", "Gps status changed event " + e.getMessage());
                }
                break;
        }
    }
}

