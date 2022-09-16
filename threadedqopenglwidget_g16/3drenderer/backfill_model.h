#include "base_model.h"
class BackFillModel : public BaseModel
{
public:
	BackFillModel();
	~BackFillModel();

	void Init();
	void InitStack(float VOL_X1, float VOL_X0, float VOL_Y1, float VOL_Y0, float VOL_Z1, float VOL_Z0);
	void Render();

	//设置中心位置
	void SetPos(vec3 pos);
	vec3 GetPos();
	//更新modelmatrix
	void rotateModelMatrix(mat4 rotatemat);
	void CalModelMatrix(const BoundingBox BB);
	mat4 GetModelMatrix();
	//
	void UpdateData();

private:
	int point_num;
	vec3 center_Pos;
};
