// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.preferences;

import android.preference.Preference;
import android.view.View;

import org.chromium.chrome.browser.util.ViewUtils;

/**
 * A delegate that determines whether a Preference is managed by enterprise policy. This is used
 * in various Preference subclasses (e.g. ChromeSwitchPreference) to determine whether to show
 * an enterprise icon next to the Preference and whether to disable clicks on the Preference.
 *
 * An implementation of this delegate should override isPreferenceControlledByPolicy() and,
 * optionally, isPreferenceClickDisabledByPolicy(). Example:
 *
 *   class RocketManagedPreferenceDelegate extends ManagedPreferenceDelegate {
 *       @Override
 *       public boolean isPreferenceControlledByPolicy(Preference preference) {
 *           if ("enable_rockets".equals(preference.getKey())) {
 *               return RocketUtils.isEnableRocketsManaged();
 *           }
 *           return false;
 *       }
 *   }
 *
 *   ChromeSwitchPreference enableRocketsPref = ...;
 *   enableRocketsPref.setManagedPreferenceDelegate(new RocketManagedPreferenceDelegate());
 */
public abstract class ManagedPreferenceDelegate {
    /**
     * Returns whether the given Preference is controlled by an enterprise policy.
     * @param preference the {@link Preference} under consideration.
     * @return whether the given Preference is controlled by an enterprise policy.
     */
    public abstract boolean isPreferenceControlledByPolicy(Preference preference);

    /**
     * Returns whether the given Preference is controlled by the supervised user's custodian.
     * @param preference the {@link Preference} under consideration.
     * @return whether the given Preference is controlled by the supervised user's custodian.
     */
    public boolean isPreferenceControlledByCustodian(Preference preference) {
        return false;
    }

    /**
     * Returns whether clicking on the given Preference is disabled due to a policy. The default
     * implementation just returns whether the preference is not modifiable by the user.
     * However, some preferences that are controlled by policy may still be clicked to show an
     * informational subscreen, in which case this method needs a custom implementation.
     */
    // TODO(bauerb): Rename to isPreferenceClickDisabled.
    public boolean isPreferenceClickDisabledByPolicy(Preference preference) {
        return !isPreferenceUserModifiable(preference);
    }

    private boolean isPreferenceUserModifiable(Preference preference) {
        return !isPreferenceControlledByPolicy(preference)
                && !isPreferenceControlledByCustodian(preference);
    }

    /**
     * Initializes the Preference based on the state of any policies that may affect it,
     * e.g. by showing a managed icon or disabling clicks on the preference.
     *
     * This should be called once, before the preference is displayed.
     */
    public void initPreference(Preference preference) {
        if (isPreferenceControlledByPolicy(preference)) {
            preference.setIcon(ManagedPreferencesUtils.getManagedByEnterpriseIconId());
        }

        if (isPreferenceClickDisabledByPolicy(preference)) {
            // Disable the views and prevent the Preference from mucking with the enabled state.
            preference.setShouldDisableView(false);

            // Prevent default click behavior.
            preference.setFragment(null);
            preference.setIntent(null);
            preference.setOnPreferenceClickListener(null);
        }
    }

    /**
     * Disables the Preference's views if the preference is not clickable.
     *
     * Note: this disables the View instead of disabling the Preference, so that the Preference
     * still receives click events, which will trigger a "Managed by your administrator" toast.
     *
     * This should be called from the Preference's onBindView() method.
     *
     * @param preference The Preference that owns the view
     * @param view The View that was bound to the Preference
     */
    public void onBindViewToPreference(Preference preference, View view) {
        if (isPreferenceClickDisabledByPolicy(preference)) {
            ViewUtils.setEnabledRecursive(view, false);
        }
    }

    /**
     * Intercepts the click event if the given Preference is managed and shows a toast in that case.
     *
     * This should be called from the Preference's onClick() method.
     *
     * @param preference The Preference that was clicked.
     * @return true if the click event was handled by this helper and shouldn't be further
     *         propagated; false otherwise.
     */
    public boolean onClickPreference(Preference preference) {
        if (!isPreferenceClickDisabledByPolicy(preference)) return false;

        if (isPreferenceControlledByPolicy(preference)) {
            ManagedPreferencesUtils.showManagedByAdministratorToast(preference.getContext());
        } else if (isPreferenceControlledByCustodian(preference)) {
            ManagedPreferencesUtils.showManagedByParentToast(preference.getContext());
        } else {
            // If the preference is disabled, it should be either because it's managed by enterprise
            // policy or by the custodian.
            assert false;
        }
        return true;
    }
}