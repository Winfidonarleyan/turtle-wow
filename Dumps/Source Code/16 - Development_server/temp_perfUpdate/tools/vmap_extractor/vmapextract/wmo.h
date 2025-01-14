/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef WMO_H
#define WMO_H
#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)

#include <string>
#include <set>
#include <vector>
#include "vec3d.h"
#include "loadlib/loadlib.h"

// MOPY flags
#define WMO_MATERIAL_NOCAMCOLLIDE    0x01
#define WMO_MATERIAL_DETAIL          0x02
#define WMO_MATERIAL_NO_COLLISION    0x04
#define WMO_MATERIAL_HINT            0x08
#define WMO_MATERIAL_RENDER          0x10
#define WMO_MATERIAL_COLLIDE_HIT     0x20
#define WMO_MATERIAL_WALL_SURFACE    0x40

class WMOInstance;
class WMOManager;
class MPQFile;
class Model;

/* for whatever reason a certain company just can't stick to one coordinate system... */
static inline Vec3D fixCoords(const Vec3D& v) { return Vec3D(v.z, v.x, v.y); }

class WMOModelInstance
{
public:
    // header
    Vec3D pos;              // Position
    float w;                // W for Quat Rotation
    Vec3D dir;              // Direction for Quat Rotation
    float sc;               // Scale Factor
    unsigned int d1;

    std::string filename;
    int id;
    unsigned int scale;
    int light;
    Vec3D ldir;
    Vec3D lcol;
    Model* model;

    WMOModelInstance() {}
    void init(std::string fname, MPQFile &f);
};


class WMORoot
{
    public:
        uint32 nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, RootWMOID, liquidType;
        unsigned int col;
        float bbcorn1[3];
        float bbcorn2[3];

        WMORoot(std::string& filename);
        ~WMORoot();

        bool open();
        bool ConvertToVMAPRootWmo(FILE* output);
        Model* GetDoodadModel(unsigned int i)
        {
            if (i >= nModels)
                return NULL;
            return modelis[i]->model;
        }
    private:
        std::string filename;
        char outfilename;
        std::vector<std::string> doodadModels;
        WMOModelInstance** modelis;
};

struct WMOLiquidHeader
{
    int xverts, yverts, xtiles, ytiles;
    float pos_x;
    float pos_y;
    float pos_z;
    short type;
};

struct WMOLiquidVert
{
    uint16 unk1;
    uint16 unk2;
    float height;
};

struct WMODoodadSet
{
    char name[0x14]; // set name
    int start; // index of first doodad instance in this set
    uint32 size; // number of doodad instances in this set
    int unused; // unused? (always 0)
};

class WMOGroup
{
    public:
        // MOGP
        int groupName, descGroupName, mogpFlags;
        float bbcorn1[3];
        float bbcorn2[3];
        uint16 moprIdx;
        uint16 moprNItems;
        uint16 nBatchA;
        uint16 nBatchB;
        uint32 nBatchC, fogIdx, liquidType, groupWMOID;

        int mopy_size, moba_size;
        int LiquEx_size;
        unsigned int nVertices; // number when loaded
        int nTriangles; // number when loaded
        char* MOPY;
        uint16* MOVI;
        uint16* MoviEx;
        float* MOVT;
        uint16* MOBA;
        int* MobaEx;
        WMOLiquidHeader* hlq;
        WMOLiquidVert* LiquEx;
        char* LiquBytes;
        uint32 liquflags;

        int nDoodads = 0;
        short* doodads;

        WMOGroup(std::string& filename, WMORoot* root);
        ~WMOGroup();

        bool open();
        int ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool pPreciseVectorData);
        void WriteDoodadsTriangles(FILE* output, int indexShift);
        void WriteDoodadsVertices(FILE* output);
    private:
        std::string filename;
        char outfilename;
        WMORoot* root;
};

class WMOInstance
{
        static std::set<int> ids;
    public:
        std::string MapName;
        int currx;
        int curry;
        WMOGroup* wmo;
        Vec3D pos;
        Vec3D pos2, pos3, rot;
        uint32 indx, id, d2, d3;
        int doodadset;

        WMOInstance(MPQFile& f, const char* WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile);

        static void reset();
};

#endif
