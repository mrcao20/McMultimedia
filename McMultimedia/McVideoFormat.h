#ifndef _MC_VIDEO_FORMAT_H_
#define _MC_VIDEO_FORMAT_H_

class McVideoFormat {
public:
	enum PixelFormat {
		Format_Invalid = -1,

		Format_ARGB,	// AARRGGBB or 00RRGGBB, check hasAlpha is required
		Format_BGRA,	// BBGGRRAA
		Format_RGB32,	// 0xAARRGGBB native endian. same as QImage::Format_ARGB32. be: ARGB, le: BGRA

		Format_YUV420P
	};
};

#endif // !_MC_VIDEO_FORMAT_H_

