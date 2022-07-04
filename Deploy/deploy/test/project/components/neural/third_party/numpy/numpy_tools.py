# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.15.
# ================================================================================
#
import numpy as np


def numpy_tools_cmp_array(array_1, array_2):
    if len(array_1) < 5 or len(array_2) < 5:
        return False

    if array_1.shape == array_2.shape:
        return np.array_equal(array_1, array_2)
    else:
        return False

def numpy_tools_compute_cosin_distance(Q, feats, names, k=None):
    if k is None:
        k = len(feats)
    dists = np.dot(Q, feats.T)
    idxs = np.argsort(dists)[::-1]
    rank_dists = dists[idxs]
    rank_names = [names[k] for k in idxs]
    rank_names = rank_names[0:k]
    return (idxs[:k], rank_dists[:k], rank_names)