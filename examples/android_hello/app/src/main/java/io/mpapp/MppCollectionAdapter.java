// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0028 — RecyclerView.Adapter that backs the Android
// collection_view real handler. Single Adapter handles both rendering
// pipelines:
//
//   * String mode: `setStrings(...)` populates with a String[]; each
//     ViewHolder shows a plain TextView. Used when the surface's
//     items_source is set (and no typed_items / item_template wins).
//
//   * Native-view mode: `setNativeViews(...)` populates with a View[]
//     of pre-built native Android views (typically materialized
//     by item_template or supplied via typed_items). Each VH wraps
//     the supplied view inside its own FrameLayout slot.
//
// The active LayoutManager (LinearLayoutManager or GridLayoutManager,
// vertical or horizontal) is set externally by the C++ handler in
// response to `collection_view::layout` — see apply_layout.
//
// Selection: the Adapter owns the source-of-truth `checked` set and
// pushes changes back to native via MppItemClickRouter's package-
// private JNI hooks. Single-select mode auto-clears siblings on tap;
// multi-select mode toggles. selection_mode = none disables all
// selection updates.

package io.mpapp;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.Arrays;
import java.util.HashSet;

public final class MppCollectionAdapter
        extends RecyclerView.Adapter<MppCollectionAdapter.VH> {

    // Choice modes — kept numerically aligned with android.widget.AbsListView
    // so the C++ handler doesn't need a separate enum.
    public static final int MODE_NONE     = 0;
    public static final int MODE_SINGLE   = 1;
    public static final int MODE_MULTIPLE = 2;

    private final long ownerPtr;
    private final int  kind = 1;  // collection_view (see MppItemClickRouter doc)

    private String[]      strings = new String[0];
    private View[]        views   = null;   // null ⇒ string mode
    private final HashSet<Integer> checked = new HashSet<>();
    private int           selectionMode    = MODE_SINGLE;

    public MppCollectionAdapter(long ownerPtr) {
        this.ownerPtr = ownerPtr;
        setHasStableIds(true);
    }

    public void setStrings(String[] s) {
        this.strings = (s != null) ? s : new String[0];
        this.views   = null;
        notifyDataSetChanged();
    }

    public void setNativeViews(View[] v) {
        this.views   = v;
        this.strings = new String[0];
        notifyDataSetChanged();
    }

    public void setSelectionMode(int mode) {
        this.selectionMode = mode;
        if (mode == MODE_NONE) {
            checked.clear();
            notifyDataSetChanged();
        }
    }

    // Programmatic single-selection from C++. Multi-select uses
    // setCheckedSet so the adapter knows the full set at once.
    public void selectIndex(int idx) {
        checked.clear();
        if (idx >= 0) checked.add(idx);
        notifyDataSetChanged();
    }

    public void setCheckedSet(int[] positions) {
        checked.clear();
        if (positions != null) {
            for (int p : positions) checked.add(p);
        }
        notifyDataSetChanged();
    }

    @Override public long getItemId(int position) { return position; }

    @Override
    public int getItemCount() {
        return (views != null) ? views.length : strings.length;
    }

    @NonNull
    @Override
    public VH onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        Context ctx = parent.getContext();
        FrameLayout fl = new FrameLayout(ctx);
        fl.setLayoutParams(new RecyclerView.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        fl.setPadding(16, 16, 16, 16);
        return new VH(fl);
    }

    @Override
    public void onBindViewHolder(@NonNull VH vh, int position) {
        FrameLayout fl = (FrameLayout) vh.itemView;
        fl.removeAllViews();
        if (views != null && position < views.length && views[position] != null) {
            View v = views[position];
            // Detach the native view from any prior FrameLayout parent
            // before re-attaching — non-virtualizing path (each position
            // owns one persistent View) means parent re-binds happen on
            // every notifyDataSetChanged.
            ViewGroup oldParent = (v.getParent() instanceof ViewGroup)
                                  ? (ViewGroup) v.getParent() : null;
            if (oldParent != null) oldParent.removeView(v);
            fl.addView(v);
        } else if (position < strings.length) {
            TextView tv = new TextView(fl.getContext());
            tv.setText(strings[position]);
            tv.setTextSize(16);
            fl.addView(tv);
        }
        fl.setActivated(checked.contains(position));
        final int pos = position;
        fl.setOnClickListener(view -> handleTap(pos));
    }

    private void handleTap(int position) {
        if (selectionMode == MODE_SINGLE) {
            // Clear all then set this. Re-bind everyone so the activated
            // state updates visually (RecyclerView won't re-bind
            // unchanged positions otherwise).
            checked.clear();
            checked.add(position);
            notifyDataSetChanged();
        } else if (selectionMode == MODE_MULTIPLE) {
            // Toggle membership.
            if (!checked.add(position)) checked.remove(position);
            notifyItemChanged(position);
            int[] arr = new int[checked.size()];
            int i = 0;
            for (Integer k : checked) arr[i++] = k;
            Arrays.sort(arr);
            MppItemClickRouter.nativeDispatchCheckedSet(ownerPtr, kind, arr);
        }
        // Single-shot tap signal — fires regardless of selection_mode so
        // item_tapped works in mode_none too (consistent with other
        // platforms).
        MppItemClickRouter.nativeDispatchItemClick(ownerPtr, kind, position);
    }

    public static final class VH extends RecyclerView.ViewHolder {
        public VH(@NonNull View itemView) { super(itemView); }
    }
}
