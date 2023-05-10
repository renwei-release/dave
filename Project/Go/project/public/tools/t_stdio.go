package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

func T_stdio_remove_string_duplicates(strArray []string) []string {  
	keys := make(map[string]bool)
	uniqueArray := []string{}
  
	for _, entry := range strArray {  
		if _, value := keys[entry]; !value {  
			keys[entry] = true  
			uniqueArray = append(uniqueArray, entry)  
		}  
	}  

	return uniqueArray  
}