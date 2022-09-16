#pragma once
#include "base_model.h"
class CoreModel : public BaseModel
{
public:
	CoreModel();
	~CoreModel();

	void Init();

	//受clipping功能影响(clipping)

	//ts,th,tw是三个方向，压缩之后的实际长度与补齐(补齐至2的整数次方)之后的比值。
	//(clipping)沿z方向绘制时，s0是clipping绘制z轴的起点，s1是z轴终点，h0是y轴起点，h1是y轴终点，w0是x轴起点，w1是x轴终点(全部归一化至0-1，clipping按比例影响)。沿不同方向代表轴有所变化
	//ds是片间距，根据不同绘制方向会有变化(片数不同，总长是1)，
	//(clipping)slice0是在该方向的切片起始，slice1是在该方向的切片结束(操作哪个方向，slice表示哪个方向)
	//thickness是厚度(至少是间隔是1，不同方向厚度不同，z方向是step / pixelSize)
	//tex3D是3d纹理，texs是2d纹理，stack_i是贴图方向
	//direction是绘制方向(从前向后还是从后向前,受深度测试影响)
	//section是cross-section模式下的，由render_mode决定
	//b_tex3d是否启用3d纹理
	//b_stream
	void InitStack();
	void Render();
	void UpdateParam(double ts, double th, double tw,
		double s0, double s1, double h0, double h1, double w0, double w1,
		double ds, int slice0, int slice1, int thickness,
		GLuint texs[], int stack_i,
		float direction, int section, int channels);

	//设置中心位置
	void SetPos(vec3 pos);
	vec3 GetPos();
	//设置纹理
	void SetTextureId(GLuint textureid);
	void SetTextureId(GLuint textureid, int channel);
	GLuint GetTextureId();
	GLuint GetTextureId(int channel);
	//更新modelmatrix
	void rotateModelMatrix(mat4 rotatemat);
	void CalModelMatrix(const BoundingBox BB);
	mat4 GetModelMatrix();

	void SetTexID(unsigned int textID);
	unsigned int GetTexID();

private:
	double ts, th, tw;
	double s0, s1, h0, h1, w0, w1;
	double ds;
	int slice0, slice1;
	int thickness;
	GLuint *texs;
	int stack_i;
	float direction;
	int section;
	bool b_tex3d;
	bool b_stream;
	int channels;

	//物体中心坐标
	vec3 center_Pos;
	//纹理数组编号
	GLuint textureArrayId_rgb;
	GLuint textureArrayId[3];
};

