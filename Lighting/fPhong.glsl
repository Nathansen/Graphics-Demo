﻿#version 330 core
const int LightNum = 3; // 光源数量

in vec3 fN;	// 法向(观察坐标系)
in vec3 fE;	// 观察向量(观察坐标系)
in vec3 fL[LightNum];	// 光照向量(观察坐标系)
in float dist;	// 片元到手电筒光源距离
in vec2 texCoord;	// 输入片元纹理坐标

uniform sampler2D tex;	 // 2D纹理采样器

uniform bool LightOn[LightNum];	// 光源开关

// 最后一个光源的聚光灯参数
uniform vec3 SpotDirection;   // 聚光灯照射方向(观察坐标系)
uniform float SpotCutOff;	  // 聚光灯截止角(角度)
uniform float SpotExponent;   // 聚光灯衰减指数

// 三种物体材质的反射系数和对应光照分量的乘积
uniform vec4 AmbientProduct[LightNum];
uniform vec4 DiffuseProduct[LightNum]; 
uniform vec4 SpecularProduct[LightNum];
uniform vec4 Emission;		// 发射光
uniform float Shininess;	// 高光系数
uniform bool bOnlyTexture; //是否只用纹理

out vec4 fragColor;		 // 输出片元颜色

void main()
{
	if(bOnlyTexture)
		fragColor = texture2D(tex, texCoord);
	else{
			// 归一化输入的向量
	vec3 N = normalize(fN);
	vec3 E = normalize(fE);
		
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0); //镜面光分量
		
	fragColor = vec4(0.0, 0.0, 0.0, 0.0); // 初始为0，针对ATI显卡
	for(int i = 0; i < LightNum; i++){	
		if(!LightOn[i]) continue; // 光源关闭，则不计算该光源的贡献
			
		vec3 L = normalize(fL[i]); // 光源方向向量(从顶点指向光源)
			
		float KSpot = 1.0;	// 受聚光灯影响的衰减系数(1.0即不衰减)
		if(i == 2){
			// 对照射方向归一化并反向(因为L也是从顶点指向光源)
			vec3 spotDir = -normalize(SpotDirection); 
			float cutoff = radians(SpotCutOff); // 角度转弧度
			float c = dot(L, spotDir);	// 偏离角的cos值
			if( c < cos(cutoff)) // 偏离角度超过截止角
				KSpot = 0.0;	// 完全衰减
			else {// 强度衰减正比于c^f
				// d为随距离衰减公式的分母
				float d = 1.0 + 0.5 * dist; 
				KSpot = max(pow(c, SpotExponent), 0) / d;
			}
		}
			
		vec3 H = normalize( L + E );	// 半角向量

		// 环境反射分量
		vec4 ambient = AmbientProduct[i];

		// 漫反射分量
		float Kd = max( dot(L, N), 0.0 );
		vec4 diffuse = KSpot * Kd * DiffuseProduct[i];
		
			// 镜面反射分量
			//vec4 specular;
			//if( Kd == 0 ) {  // 即dot(L, N) <= 0
			//	specular = vec4(0.0, 0.0, 0.0, 1.0);
			//} 
			//else{
			//	float Ks = pow( max(dot(N, H), 0.0), Shininess );
			//	specular = KSpot * Ks * SpecularProduct[i];
			//}
			// 镜面反射分量
		if( Kd != 0 ) {  // 即dot(L, N) > 0
			float Ks = pow( max(dot(N, H), 0.0), Shininess );
			specular += KSpot * Ks * SpecularProduct[i]; // 镜面分量单独累加
		} 

		// 得到最终颜色(不包含镜面光)
		fragColor += ambient + diffuse; 
	}
		
	fragColor += Emission;	// 加上物体自身的发射光
		fragColor = fragColor * texture2D(tex, texCoord) + specular; // 得到最终片元颜色
	
		// 设置透明度
		fragColor.a = (AmbientProduct[0].a + DiffuseProduct[0].a + SpecularProduct[0].a) / 3;
	}
}
