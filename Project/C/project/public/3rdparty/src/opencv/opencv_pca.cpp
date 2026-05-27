/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * https://blog.csdn.net/guofei_fly/article/details/103956940
 * ================================================================================
 */

#include "3rdparty_macro.h"
#if defined(OPENCV_3RDPARTY)
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include "opencv2/core/core_c.h"
#include <vector>
#include <new>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "dave_os.h"
#include "dave_define.h"
#include "dave_toolbox.h"
#include "dave_tools.h"
#include "party_log.h"

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

// =====================================================================

extern "C" dave_bool
dave_opencv_cvCalcPCA(void *pMean, void *pEigVals, void *pEigVecs, void *pData)
{
	cvCalcPCA((CvMat *)pData, (CvMat *)pMean, (CvMat *)pEigVals, (CvMat *)pEigVecs, CV_PCA_DATA_AS_ROW);

	return dave_true;
}

extern "C" dave_bool
dave_opencv_cvProjectPCA(void *pPCA, void *pData, void *pMean, void *pEigVecs)
{
	cvProjectPCA((CvMat *)pData, (CvMat *)pMean, (CvMat *)pEigVecs, (CvMat *)pPCA);

	return dave_true;
}

#endif

