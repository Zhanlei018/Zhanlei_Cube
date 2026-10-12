/**
 * min2phaseCXX Copyright (C) 2022 Borgo Federico
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 *
 * ---------------------------------------------------------------------------
 * 【本项目改动说明】
 *
 * 这份文件是上游 src/min2phase.cpp 的【裁剪版】：
 *   · 删掉了 server() / stop() / webSearch() 三个函数
 *   · 因此不再 include "http.h"，也就不需要链接 pthread
 *
 * 原因：上游那个 HTTP server 是给"把求解器当服务跑"用的，
 * 我们是在魔方程序里按 Q 键本地求解，完全用不到，
 * 而它会把 pthread 依赖拖进来。
 *
 * 保留的公开接口：init() / writeFile() / loadFile() / solve()
 * 其余部分（算法本体）一行未改。
 *
 * 文件名说明：上游叫 min2phase.cpp，这里叫 api.cpp 只是为了不跟
 * 目录名 min2phase/ 混在一起读起来别扭，内容是一样的。
 * ---------------------------------------------------------------------------
 */

#include <fstream>

#include <min2phase/min2phase.h>

#include "Search.h"

namespace min2phase {

    //initialization
    void init() {
        info::init();
        coords::init();
    }

    //write coords
    bool writeFile(const std::string& name){
        if(!coords::isInit())
            init();

        std::ofstream out(name);

        MIN2PHASE_OUTPUT("Writing file.")

        if(out) {
            out.write(reinterpret_cast<char *>(&coords::coords), sizeof(coords::coords_t));
            out.close();
            return true;
        }

        return false;
    }

    //load coords
    bool loadFile(const std::string& name){
        std::ifstream in(name);

        MIN2PHASE_OUTPUT("Loading file.")

        if(in){
            in.read(reinterpret_cast<char*>(&coords::coords), sizeof(coords::coords_t));
            in.close();
            info::init();
            return true;
        }
        else
            init();

        return false;
    }

    //solve the cube
    std::string solve(const std::string &facelets, int8_t maxDepth, int32_t probeMax, int32_t probeMin,
                                     int8_t verbose, uint8_t* usedMoves) {
        MIN2PHASE_OUTPUT("Init solving.")
        return Search().solve(facelets, maxDepth, probeMax, probeMin, verbose, usedMoves);
    }
}
