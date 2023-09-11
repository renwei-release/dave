# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.28.
# faiss的代码来自一下链接：
# https://github.com/facebookresearch/faiss
#
# 运行容器环境cuda11
#
# https://zhuanlan.zhihu.com/p/364923722
# https://zhuanlan.zhihu.com/p/65334855
# https://www.cnblogs.com/yhzhou/p/10568728.html
# https://www.cnblogs.com/houkai/p/9316155.html
#
# faiss 和 scnn在应用上的区别  ： 训练的类别数 scnn测试没有变化 faiss较大时效果不理想
#                              测试返回最大topN数量，scnn最大有top100，faiss没有限制
# 索引名	类名	index_factory	主要参数	字节数/向量	精准检索	备注
# 精准的L2搜索	IndexFlatL2	"Flat"	d	4*d	yes	brute-force
# 精准的内积搜索	IndexFlatIP	"Flat"	d	4*d	yes	归一化向量计算cos
# Hierarchical Navigable Small World graph exploration	IndexHNSWFlat	"HNSWx,Flat"	d, M	4*d + 8 * M	no	-
# 倒排文件	IndexIVFFlat	"IVFx,Flat"	quantizer, d, nlists, metric	4*d	no	需要另一个量化器来建立倒排
# Locality-Sensitive Hashing (binary flat index)	IndexLSH	-	d, nbits	nbits/8	yes	optimized by using random rotation instead of random projections
# Scalar quantizer (SQ) in flat mode	IndexScalarQuantizer	"SQ8"	d	d	yes	每个维度项可以用4 bit表示，但是精度会受到一定影响
# Product quantizer (PQ) in flat mode	IndexPQ	"PQx"	d, M, nbits	M (if nbits=8)	yes	-
# IVF and scalar quantizer	IndexIVFScalarQuantizer	"IVFx,SQ4" "IVFx,SQ8"	quantizer, d, nlists, qtype	d or d/2	no	有两种编码方式：每个维度项4bit或8bit
# IVFADC (coarse quantizer+PQ on residuals)	IndexIVFPQ	"IVFx,PQy"	quantizer, d, nlists, M, nbits	M+4 or M+8	no	内存和数据id（int、long）相关，目前只支持 nbits <= 8
# IVFADC+R (same as IVFADC with re-ranking based on codes)	IndexIVFPQR	"IVFx,PQy+z"	quantizer, d, nlists, M, nbits, M_refine, nbits_refine	M+M_refine+4 or M+M_refine+8	no
# ================================================================================
#
import faiss
import sys
import numpy as np
from components.neural.neural_tools.picture.picture_feature import t_picture_feature_array
from public.tools import *


def _correction_number_of_cluster_centers(number_of_cluster_centers, file_number):
    if file_number < number_of_cluster_centers:
        number_of_cluster_centers = int(file_number / 16)
    if number_of_cluster_centers <= 0:
        number_of_cluster_centers = 1
    return number_of_cluster_centers


def _setup_faiss_index(feature_dimension, number_of_cluster_centers, file_number):
    number_of_cluster_centers = _correction_number_of_cluster_centers(number_of_cluster_centers, file_number)
    #
    # IndexFlatL2
    # IndexFlatIP
    # IndexHNSWFlat
    # IndexIVFFlat
    # IndexLSH
    # IndexScalarQuantizer
    # IndexPQ
    # IndexIVFScalarQuantizer
    # IndexIVFPQ
    # IndexIVFPQR
    #
    # 工厂方法创建索引
    # index = faiss.index_factory(d,"PCA32,IVF100,PQ8 ")
    # 使用PCA算法将向量降维到32维, 划分成100个nprobe (搜索空间), 通过PQ算法将每个向量压缩成8bit。
    #
    quantizer = faiss.IndexFlatIP(feature_dimension)
    index = faiss.IndexIVFFlat(quantizer, feature_dimension, number_of_cluster_centers, faiss.METRIC_INNER_PRODUCT)
    return index


def _model_train(faiss_index, feature_array):
    feature_array = np.array(feature_array).astype('float32')
    assert not faiss_index.is_trained
    faiss_index.train(feature_array)
    assert faiss_index.is_trained
    faiss_index.add(feature_array)
    return faiss_index


def _save_ids(feature_ids, model_path):
    ids_file = model_path + '/ids.idtable'
    t_dict_save(ids_file, feature_ids)
    return


def _save_model(faiss_index, model_path):
    faiss.write_index(faiss_index, model_path+'/model.index')
    return


def _save_feature(feature_ids, feature_array, model_path):
    feature_and_ids_array = {}

    for feature_index, feature_data in enumerate(feature_array):
        file_id = feature_ids[feature_index].rsplit('/', 1)[-1].rsplit('.', 1)[0]
        feature_data = feature_data.tolist()
        feature_and_ids_array[file_id] = feature_data

    ids_file = model_path + '/feature.idtable'
    t_dict_save(ids_file, feature_and_ids_array)
    return


def _save_result(faiss_index, feature_ids, feature_array, model_path):
    _save_model(faiss_index, model_path)
    _save_ids(feature_ids, model_path)
    _save_feature(feature_ids, feature_array, model_path)
    return


# =====================================================================


def train(
        train_path = '/project/dataset/Private/PhotographicAesthetics/train',
        feature_model = 'pose'
    ):
    number_of_cluster_centers = 10

    file_number, feature_array, feature_ids, feature_dimension, feature_name = t_picture_feature_array(train_path, feature_model)
    if file_number != None:
        model_path = t_train_path_to_model_path(train_path, 'faiss', None, None, feature_name+'_'+str(file_number))
        faiss_index = _setup_faiss_index(feature_dimension, number_of_cluster_centers, file_number)
        faiss_index = _model_train(faiss_index, feature_array)
        _save_result(faiss_index, feature_ids, feature_array, model_path)
        print(f'model_path:{model_path} feature_array:{len(feature_array)}')

    else:
        model_path = None
        print(f'invalid train_path:{train_path}')
    return model_path


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        train()
    else:
        train(train_path=sys.argv[1], feature_model=sys.argv[2])