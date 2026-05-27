# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os

#
# reference:
# https://github.com/huggingface/transformers/blob/main/src/transformers/models/llama/convert_llama_weights_to_hf.py
#


# =====================================================================


if __name__ == "__main__":
    input_dir = "/dave/llm/llama3/Meta-Llama-3-70B"
    model_size = "70B"
    output_dir = f"{input_dir}/huggingface"
    os.system(f"python3 convert_llama_weights_to_hf.py --input_dir {input_dir} --model_size {model_size} --output_dir {output_dir}")