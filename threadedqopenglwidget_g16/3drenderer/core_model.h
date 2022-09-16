#pragma once
#include "base_model.h"
class CoreModel : public BaseModel
{
public:
	CoreModel();
	~CoreModel();

	void Init();

	//��clipping����Ӱ��(clipping)

	//ts,th,tw����������ѹ��֮���ʵ�ʳ����벹��(������2�������η�)֮��ı�ֵ��
	//(clipping)��z�������ʱ��s0��clipping����z�����㣬s1��z���յ㣬h0��y����㣬h1��y���յ㣬w0��x����㣬w1��x���յ�(ȫ����һ����0-1��clipping������Ӱ��)���ز�ͬ��������������仯
	//ds��Ƭ��࣬���ݲ�ͬ���Ʒ�����б仯(Ƭ����ͬ���ܳ���1)��
	//(clipping)slice0���ڸ÷������Ƭ��ʼ��slice1���ڸ÷������Ƭ����(�����ĸ�����slice��ʾ�ĸ�����)
	//thickness�Ǻ��(�����Ǽ����1����ͬ�����Ȳ�ͬ��z������step / pixelSize)
	//tex3D��3d����texs��2d����stack_i����ͼ����
	//direction�ǻ��Ʒ���(��ǰ����ǴӺ���ǰ,����Ȳ���Ӱ��)
	//section��cross-sectionģʽ�µģ���render_mode����
	//b_tex3d�Ƿ�����3d����
	//b_stream
	void InitStack();
	void Render();
	void UpdateParam(double ts, double th, double tw,
		double s0, double s1, double h0, double h1, double w0, double w1,
		double ds, int slice0, int slice1, int thickness,
		GLuint texs[], int stack_i,
		float direction, int section, int channels);

	//��������λ��
	void SetPos(vec3 pos);
	vec3 GetPos();
	//��������
	void SetTextureId(GLuint textureid);
	void SetTextureId(GLuint textureid, int channel);
	GLuint GetTextureId();
	GLuint GetTextureId(int channel);
	//����modelmatrix
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

	//������������
	vec3 center_Pos;
	//����������
	GLuint textureArrayId_rgb;
	GLuint textureArrayId[3];
};

