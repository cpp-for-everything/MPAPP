// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. AdapterView.OnItemClickListener that dispatches the
// (position) of the tapped row back into native C++ via JNI.
//
// AdapterView's onItemClick callback has a different signature from
// View.OnClickListener.onClick, so this is a separate router class
// from MppActionRouter.
//
// Kinds defined today (kept in sync with src/handlers/android/item_click_router.cpp):
//   0 — ListView       (ownerPtr = list_view*, payload = row index)
//   1 — CollectionView (ownerPtr = collection_view*, payload = row index)

package io.mpapp;

import android.view.View;
import android.widget.AdapterView;

public final class MppItemClickRouter implements AdapterView.OnItemClickListener {
    private final long ownerPtr;
    private final int  kind;

    public MppItemClickRouter(long ownerPtr, int kind) {
        this.ownerPtr = ownerPtr;
        this.kind     = kind;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        nativeDispatchItemClick(ownerPtr, kind, position);
    }

    private static native void nativeDispatchItemClick(long ownerPtr, int kind, int position);
}
