/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OPENCV_MAGSAC_HPP__
#define __OPENCV_MAGSAC_HPP__

void magsac_findHomography(ub *inliners_size, ub *inliners_match_size, vector<DMatch> &matches, std::vector<cv::KeyPoint> &keypoints_a, std::vector<cv::KeyPoint> &keypoints_b);

#endif

