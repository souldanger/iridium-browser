// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview 'settings-search-engines-list' is a component for showing a
 * list of search engines.
 */
Polymer({
  is: 'settings-search-engines-list',

  behaviors: [settings.GlobalScrollTargetBehavior],

  properties: {
    /** @type {!Array<!SearchEngine>} */
    engines: Array,

    /** @override */
    subpageRoute: {
      type: Object,
      value: settings.Route.SEARCH_ENGINES,
    },

    /** Whether column headers should be displayed */
    hideHeaders: Boolean,
  },
});
