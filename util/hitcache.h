 /*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * @FileName: HitCache.h
 * @Description: TODO
 * All rights Reserved, Designed By huih
 * @Company: onexsoft
 * @Author: hui
 * @Version: V1.0
 * @Date: 2017年1月17日 下午3:36:56
 *  
 */

#ifndef UTIL_HITCACHE_H_
#define UTIL_HITCACHE_H_

class HitCache{
#define CACHE_MAP_COLOMNS 130
#define CACHE_MAP_ROWS 5

public:
	HitCache(int cacheLevel = 5, bool ignoreCase = true) {
		this->isIgnoreCase = ignoreCase;
		this->cacheLevel = cacheLevel;
		if (this->cacheLevel > CACHE_MAP_ROWS) {
			this->cacheLevel = CACHE_MAP_ROWS;
		}
		for (int i = 0; i < CACHE_MAP_ROWS; ++i) {
			for (int j = 0; j < CACHE_MAP_COLOMNS; ++j) {
				this->cache[i][j] = 0;
			}
		}
	}

	void set_hitCache(const char* name, int nameLen) {
		int level = this->cacheLevel;
		if (nameLen < level)
			level = nameLen;
		if (level <= 0)
			return;

		for (int i = 0; i < level; ++i) {
			char ch = name[i];
			if (ch >= CACHE_MAP_COLOMNS)
				continue;

			if (this->isIgnoreCase) {
				if (ch >= 'A' && ch <= 'Z') {
					cache[i][(int)ch] = 1;
					cache[i][(int)ch + 32] = 1;
				} else if (ch >= 'a' && ch <= 'z') {
					cache[i][(int)ch] = 1;
					cache[i][(int)ch - 32] = 1;
				} else {
					cache[i][(int)ch] = 1;
				}
			} else {
				cache[i][(int)ch] = 1;
			}
		}
	}

	bool is_hit(const char* data, const int dataLen) {
		int level = this->cacheLevel;
		if (level > dataLen)
			level = dataLen;
		for(int i = 0; i < level; ++i) {
			int ch = (int)data[i];
			if (ch >= CACHE_MAP_COLOMNS || !cache[i][ch])
				return false;
		}
		return true;
	}


private:
	int cache[CACHE_MAP_ROWS][CACHE_MAP_COLOMNS];
	int cacheLevel;//max 5;
	bool isIgnoreCase;
};



#endif /* UTIL_HITCACHE_H_ */
