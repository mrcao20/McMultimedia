#ifndef _I_MC_VIDEO_OUTPUT_H_
#define _I_MC_VIDEO_OUTPUT_H_

class IMcVideoRenderer;

class IMcVideoOutput {
public:
	virtual ~IMcVideoOutput() = default;

	/*************************************************
	 <函数名称>		setRenderer
	 <功    能>		设置渲染器
	 <参数说明>		renderer 渲染器
	 <返回值>
	 <函数说明>		这个函数用来设置渲染器
	 <作    者>		mrcao
	 <时    间>		2019/5/6
	**************************************************/
	virtual void setRenderer(IMcVideoRenderer *renderer) noexcept = 0;
};

#endif // !_I_MC_VIDEO_OUTPUT_H_

