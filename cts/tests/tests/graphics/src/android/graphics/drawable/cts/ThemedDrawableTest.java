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

package android.graphics.drawable.cts;

import android.annotation.TargetApi;
import android.content.res.Resources.Theme;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.Shader.TileMode;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.NinePatchDrawable;
import android.graphics.drawable.RippleDrawable;
import android.os.Debug;
import android.test.AndroidTestCase;
import android.view.Gravity;

import android.graphics.cts.R;

public class ThemedDrawableTest extends AndroidTestCase {

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        // Workaround for ContextImpl.setTheme() being broken.
        final Theme theme = mContext.getResources().newTheme();
        theme.applyStyle(R.style.Theme_ThemedDrawableTest, true);
        final Theme ctxTheme = mContext.getTheme();
        ctxTheme.setTo(theme);
    }

    @Override
    public void testAndroidTestCaseSetupProperly() {
        final TypedArray t = mContext.obtainStyledAttributes(new int[]{R.attr.themeType});
        assertTrue("Theme was applied correctly", t.getInt(0, -1) == 0);
    }

    public void testBitmapDrawable() {
        BitmapDrawable d = (BitmapDrawable) mContext.getDrawable(R.drawable.bitmapdrawable_theme);

        internalTestBitmapDrawable(d);
    }

    private void internalTestBitmapDrawable(BitmapDrawable d) {
        assertEquals(true, d.hasAntiAlias());
        assertEquals(true, d.isAutoMirrored());
        // assertEquals(true, d.hasDither());
        // assertEquals(true, d.hasFilter());
        assertEquals(Gravity.TOP, d.getGravity());
        assertEquals(true, d.hasMipMap());
        assertNotNull(d.getBitmap());
        assertEquals(TileMode.MIRROR, d.getTileModeX());
        assertEquals(TileMode.MIRROR, d.getTileModeY());
    }

    public void testColorDrawable() {
        ColorDrawable d = (ColorDrawable) mContext.getDrawable(R.drawable.colordrawable_theme);

        assertEquals(Color.BLACK, d.getColor());
    }

    public void testGradientDrawable() {
        GradientDrawable d = (GradientDrawable) mContext.getDrawable(
                R.drawable.gradientdrawable_theme);

        // Corners
        // assertEquals(1, d.getCornerRadius(0));
        // assertEquals(1, d.getCornerRadius(1));
        // assertEquals(1, d.getCornerRadius(2));
        // assertEquals(1, d.getCornerRadius(3));

        // Gradient
        // int[] colors = d.getColors(null);
        // for (int i = 0; i < color.length; i++) {
        // assertEquals(Color.BLACK, colors[i]);
        // }
        // assertEquals(1.0f, d.getGradientAngle());
        // assertEquals(1.0, d.getGradientCenterX());
        // assertEquals(1.0, d.getGradientCenterY());
        // assertEquals(1.0, d.getGradientRadius());
        // assertEquals(false, d.getUseLevel());

        // Padding
        Rect padding = new Rect();
        assertTrue(d.getPadding(padding));
        assertEquals(1, padding.left);
        assertEquals(1, padding.top);
        assertEquals(1, padding.bottom);
        assertEquals(1, padding.right);

        // Size
        assertEquals(1, d.getIntrinsicHeight());
        assertEquals(1, d.getIntrinsicWidth());

        // Solid
        // assertEquals(true, d.hasSolidColor());
        // assertEquals(Color.BLACK, d.getColor());

        // Stroke
        // assertEquals(1.0, d.getStrokeWidth());
        // assertEquals(Color.BLACK, d.getStrokeColor());
        // assertEquals(1.0, d.getStrokeDashWidth());
        // assertEquals(1.0, d.getStrokeDashGap());
    }

    public void testNinePatchDrawable() {
        NinePatchDrawable d = (NinePatchDrawable) mContext.getDrawable(
                R.drawable.ninepatchdrawable_theme);

        internalTestNinePatchDrawable(d);
    }

    private void internalTestNinePatchDrawable(NinePatchDrawable d) {
        assertEquals(true, d.isAutoMirrored());
        // assertEquals(true, d.hasDither());
        // assertNotNull(d.getNinePatch());
    }

    public void testRippleDrawable() {
        RippleDrawable d = (RippleDrawable) mContext.getDrawable(
                R.drawable.rippledrawable_theme);

        // assertEquals(Color.BLACK, d.getColor());
    }

    public void testLayerDrawable() {
        LayerDrawable d = (LayerDrawable) mContext.getDrawable(R.drawable.layerdrawable_theme);

        // Layer autoMirror values are set to the parent's autoMirror value, so
        // make sure the container is using the expected value.
        assertEquals(true, d.isAutoMirrored());

        BitmapDrawable bitmapDrawable  = (BitmapDrawable) d.getDrawable(0);
        assertEquals(d, bitmapDrawable.getCallback());
        internalTestBitmapDrawable(bitmapDrawable);

        NinePatchDrawable ninePatchDrawable = (NinePatchDrawable) d.getDrawable(1);
        assertEquals(d, ninePatchDrawable.getCallback());
        internalTestNinePatchDrawable(ninePatchDrawable);

        Drawable themeDrawable = (Drawable) d.getDrawable(2);
        assertEquals(d, themeDrawable.getCallback());
    }
}
