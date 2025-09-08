//
//! Demo.fx : 기본 셰이더 소스.
//!
//! \author	김기홍 Kihong Kim / onlysonim@gmail.com 
//! \date   2003.11.04. Updated. DX9.x 
//! \date   2010.12.01. Updated. DX11, Jun.2010
//! \date   2016.12.27. Updated. DX11/12, Window SDK 8.1 / Window 10 SDK 10.0.18362
//! \date   2018.12.30. Updated. DX11.x/12.x, Windows 10 SDK 10.0.18362
//! \date   2020.08.22. Updated. DX11.x/12.x, Windows 10 SDK 10.0.19041 
//! \date   2024.12.10. Updated. DX11.x/12.x, Windows 10 SDK 10.0.22621 (VS22)
//! \date   2025.09.01. Updated. DX11.x/12.x, Windows 10 SDK 10.0.26100 (VS22)
//

// 상수 버퍼 Constant Buffer Variables
cbuffer ConstBuffer //: register(b0) //상수버퍼 0번에 등록 (최대 14개. 0~13)
{
    matrix mWorld;
    matrix mView;
    matrix mProj;
    matrix mWVP;
}




//VS 출력 구조체.
struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
};



////////////////////////////////////////////////////////////////////////////// 
//
// Vertex Shader Main : 정점 셰이더 메인 함수.
//
////////////////////////////////////////////////////////////////////////////// 

VSOutput VS_Main(
				  float4 pos : POSITION,    //[입력] 정점좌표. Vertex Position (2D Screen)
				  float4 col : COLOR0       //[입력] 정점색 Vertex Color	: "Diffuse"	
				)
{

	VSOutput o = (VSOutput)0;
    pos.w = 1.0f;

	 //변환.
    pos = mul(pos, mWVP);	 
    //pos = mul(pos, mWorld);
    //pos = mul(pos, mView);
    //pos = mul(pos, mProj);	 	

	o.pos = pos;						//변환된 정보 출력..
	o.col = col;						//색상은 변환 없이 출력.

    return o;
}





////////////////////////////////////////////////////////////////////////////// 
//
// Pixel Shader Main : 픽셀 셰이더 메인 함수.
//
////////////////////////////////////////////////////////////////////////////// 

float4 PS_Main(
				float4 pos : SV_POSITION,   //[입력] (보간된) 픽셀별 좌표. (Screen, 2D)
				float4 col : COLOR0         //[입력] (보간된) 픽셀별 색상. (Pixel Color : "Diffuse")
				) : SV_TARGET               //[출력] 색상.(필수), "렌더타겟" 으로 출력합니다.
{
	//지정색 출력.
	//float4 col2 = {1, 0, 1, 1};

    return col;
}



/**************** end of file "Demo.fx" ***********************/
