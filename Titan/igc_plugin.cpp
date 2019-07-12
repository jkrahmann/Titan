
#include <ot/igc.h>
#include <ot/fb.h>
#include <ot/sdm_types.h>
#include <ot/glm/glm_ext.h>

#include "plugin.hpp"
#include <stdarg.h>

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define VIOSO_DO

char g_logFilePath[260] = "plugins\\VIOSOWarpBlend\\VIOSOWarpBlend.log";
int g_logLevel = 3;

int logStr(int level, char const* format, ...)
{
	if (g_logLevel >= level)
	{
		FILE* f = NULL;
		if (0 == fopen_s(&f, g_logFilePath, "a+"))
		{
			char dest[1024 * 64];
			va_list params;

			va_start(params, format);
			vsprintf(dest, format, params);
			va_end(params);

			fputs(dest, f);
			fputs("\n", f);
			fclose(f);
		}
	}
	return 0;
}

#ifdef VIOSO_DO
//< start VIOSO API code
#define VIOSOWARPBLEND_DYNAMIC_DEFINE_IMPLEMENT
#include "VIOSOWarpBlend/Include/VIOSOWarpBlend.h"



#ifdef _M_IX86
char* s_warper = "plugins\\VIOSOWarpBlend\\VIOSOWarpBlend.dll";
#else
char* s_warper = "plugins\\VIOSOWarpBlend\\VIOSOWarpBlend64.dll";
#endif
char* s_configFile = "VIOSOWarpBlend.ini";
char* s_channel = "IG1";
VWB_Warper* pWarper = NULL;
//< end VIOSO API code
#endif //def VIOSO_DO

///IGC plugin class derived from ot::igc interface class
class vioso_plugin : public ot::igc
{
    bool _initialized;

    plugin plg;

public:

	vioso_plugin()
        : _initialized(false)
    {
		logStr(0, "vioso_plugin.dll was loaded.\n");
	}

    ///Invoked before rendering frame
    //@param time the current game time for the frame
    void update( double time ) override;
};


//autocreate the module on dll load
static iref<vioso_plugin> _M = ot::igc::get(new vioso_plugin);


////////////////////////////////////////////////////////////////////////////////
void vioso_plugin::update(double time)
{
    if(!_initialized) {

		logStr(0, "VIOSOWarpBlend gets initialized in update().");
	#ifdef VIOSO_DO
			//< start VIOSO API code
	#define VIOSOWARPBLEND_DYNAMIC_INITIALIZE
			#define VIOSOWARPBLEND_FILE s_warper
	#include "VIOSOWarpBlend/Include/VIOSOWarpBlend.h"

			if (NULL == VWB_Create ||
				NULL == VWB_Init ||
				NULL == VWB_getWarpBlendMesh )
			{
				logStr(0, "ERROR: vioso_plugin.dll: LoadLibrary of %s failed\n", VIOSOWARPBLEND_FILE );
				return;
			}
			VWB_ERROR err = VWB_Create(VWB_DUMMYDEVICE, s_configFile, s_channel, &pWarper, 1, NULL);
			if (VWB_ERROR_NONE != err)
			{
				logStr(0, "ERROR: vioso_plugin.dll: VWB_Create failed. It returned 0x%x.\n", err );
				return;
			}
			// maybe check and modify settings here
			err = VWB_Init(pWarper);
			if ( VWB_ERROR_NONE != err )
			{
				logStr(0, "ERROR: vioso_plugin.dll: VWB_init returned 0x%x\n", err);
				return;
			}
			logStr(2, "INFO: vioso_plugin.dll: VWB_init succeeded.\n" );
			VWB_WarpBlendMesh mesh;
			err = VWB_getWarpBlendMesh(pWarper, 21, 21, mesh);
			if (VWB_ERROR_NONE == err )
			{
				logStr(2, "INFO: vioso_plugin.dll: VWB_getWarpBlendMesh succeeded.\n");
				coid::uint nVtx = (coid::uint)mesh.nVtx;
				coid::uint nIdx = (coid::uint)mesh.nIdx;

				ot::sdm_vertex*  vtx = new ot::sdm_vertex[nVtx];
				for (coid::uint i = 0; i != nVtx; i++)
				{
					vtx[i]._vtx[0] = mesh.vtx[i].pos[0] * 2 -1;
					vtx[i]._vtx[1] = -mesh.vtx[i].pos[1] * 2 +1;
					vtx[i]._uv[0] = (uint16)MIN(65535, MAX(0, mesh.vtx[i].uv[0] * 65535 /* / mesh.dim.cx */));
					vtx[i]._uv[1] = 1 - (uint16)MIN(65535, MAX(0, mesh.vtx[i].uv[1] * 65535 /* / mesh.dim.cy */));
					float alpha = (mesh.vtx[i].rgb[0] + mesh.vtx[i].rgb[1] + mesh.vtx[i].rgb[2]) / 3.0f;
					vtx[i]._alpha = (uint(floor(alpha * 255 + 0.5f)) << 24) + 0xffffff;;
				}
				logStr(2, "INFO: vioso_plugin.dll: calculating1 mesh succeeded.\n");

				coid::uint* idx = new coid::uint[nIdx];
				logStr(2, "INFO: vioso_plugin.dll: calculating2 mesh succeeded.\n");
				for (coid::uint i = 0; i != nIdx; i++)
					idx[i] = mesh.idx[i];

				logStr(2, "INFO: vioso_plugin.dll: calculating3 mesh succeeded.\n");

				float2 fov = float2(-1, 1);

				logStr(2, "INFO: vioso_plugin.dll: calculating4 mesh succeeded.\n");
	#endif
				iref<ot::fb> x = ot::fb::get();
				logStr(2, "INFO: vioso_plugin.dll: after fb.get.\n");
	#ifdef VIOSO_DO
				x->set_sdm_mesh(vtx, nVtx, idx, nIdx, fov);
				logStr(2, "INFO: vioso_plugin.dll: set_sdm_mesh done.\n");

				delete[] vtx;
				delete[] idx;
	#endif
			}
			else
			{
				logStr(0, "ERROR: vioso_plugin.dll: VWB_getWarpBlendMesh returned 0x%x\n", err);
			}
		//< end VIOSO API code
        _initialized = true;

        return;
    }
}
