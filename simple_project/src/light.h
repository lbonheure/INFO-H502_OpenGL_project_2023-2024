#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

#include "meshes/world_transform.h"


#define NUM_POINT_LIGHTS 2
#define NUM_SPOT_LIGHTS 2


class BaseLight
{
public:
    glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;
};


class DirectionalLight : public BaseLight
{
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    void CalcLocalDirection(const WorldTrans& worldTransform)
    {
        LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
    }

    const glm::vec3& GetLocalDirection() const { return LocalDirection; }

private:
    glm::vec3 LocalDirection = glm::vec3(0.0f, 0.0f, 0.0f);
};


struct LightAttenuation
{
    float Constant = 1.0f;
    float Linear = 0.0f;
    float Exp = 0.0f;
};


class PointLight: public BaseLight
{
public:
    glm::vec3 WorldPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    LightAttenuation Attenuation;

    void CalcLocalPosition(const WorldTrans& worldTransform)
    {
        LocalPosition = worldTransform.WorldPosToLocalPos(WorldPosition);
    }

    const glm::vec3& GetLocalPosition() const { return LocalPosition; }

private:
    glm::vec3 LocalPosition = glm::vec3(0.0f, 0.0f, 0.0f);
};


class SpotLight : public PointLight
{
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    float Cutoff = 0.0f;

    void CalcLocalDirectionAndPosition(const WorldTrans& worldTransform)
    {
        CalcLocalPosition(worldTransform);

        LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
    }

    const glm::vec3& GetLocalDirection() const { return LocalDirection; }

private:
    glm::vec3 LocalDirection = glm::vec3(0.0f, 0.0f, 0.0f);
};



class Lighting
{
private:
    PointLight pointLights[NUM_POINT_LIGHTS];
    SpotLight spotLights[NUM_SPOT_LIGHTS];

    struct {
        char Color[128];
        char AmbientIntensity[128];
        char Position[128];
        char DiffuseIntensity[128];

        struct
        {
            char Constant[128];
            char Linear[128];
            char Exp[128];
        } Atten;
    } PointLightsName[NUM_POINT_LIGHTS];


    struct {
        char Color[128];
        char AmbientIntensity[128];
        char DiffuseIntensity[128];
        char Position[128];
        char Direction[128];
        char Cutoff[128];
        struct {
            char Constant[128];
            char Linear[128];
            char Exp[128];
        } Atten;
    } SpotLightsName[NUM_SPOT_LIGHTS];


    void initPointLightsName(unsigned int NumLights)
    {
        for (unsigned int i = 0 ; i < NumLights ; i++) {
            snprintf(PointLightsName[i].Color, sizeof(PointLightsName[i].Color), "gPointLights[%d].Base.Color", i);
            snprintf(PointLightsName[i].AmbientIntensity, sizeof(PointLightsName[i].AmbientIntensity), "gPointLights[%d].Base.AmbientIntensity", i);
            snprintf(PointLightsName[i].Position, sizeof(PointLightsName[i].Position), "gPointLights[%d].LocalPos", i);
            snprintf(PointLightsName[i].DiffuseIntensity, sizeof(PointLightsName[i].DiffuseIntensity), "gPointLights[%d].Base.DiffuseIntensity", i);
            snprintf(PointLightsName[i].Atten.Constant, sizeof(PointLightsName[i].Atten.Constant), "gPointLights[%d].Atten.Constant", i);
            snprintf(PointLightsName[i].Atten.Linear, sizeof(PointLightsName[i].Atten.Linear), "gPointLights[%d].Atten.Linear", i);
            snprintf(PointLightsName[i].Atten.Exp, sizeof(PointLightsName[i].Atten.Exp), "gPointLights[%d].Atten.Exp", i);
        }
    }

    void initSpotLightsName(unsigned int NumLights)
    {
        for (unsigned int i = 0 ; i < NumLights ; i++) {
            snprintf(SpotLightsName[i].Color, sizeof(SpotLightsName[i].Color), "gSpotLights[%d].Base.Color", i);
            snprintf(SpotLightsName[i].AmbientIntensity, sizeof(SpotLightsName[i].AmbientIntensity), "gSpotLights[%d].Base.AmbientIntensity", i);
            snprintf(SpotLightsName[i].Position, sizeof(SpotLightsName[i].Position), "gSpotLights[%d].LocalPos", i);
            snprintf(SpotLightsName[i].DiffuseIntensity, sizeof(SpotLightsName[i].DiffuseIntensity), "gSpotLights[%d].Base.DiffuseIntensity", i);
            snprintf(SpotLightsName[i].Direction, sizeof(SpotLightsName[i].Direction), "gSpotLights[%d].Direction", i);
            snprintf(SpotLightsName[i].Cutoff, sizeof(SpotLightsName[i].Cutoff), "gSpotLights[%d].Cutoff", i);
            snprintf(SpotLightsName[i].Atten.Constant, sizeof(SpotLightsName[i].Atten.Constant), "gSpotLights[%d].Atten.Constant", i);
            snprintf(SpotLightsName[i].Atten.Linear, sizeof(SpotLightsName[i].Atten.Linear), "gSpotLights[%d].Atten.Linear", i);
            snprintf(SpotLightsName[i].Atten.Exp, sizeof(SpotLightsName[i].Atten.Exp), "gSpotLights[%d].Atten.Exp", i);
        }
    }

    /*
    void sendDirectionalLight(const DirectionalLight& Light)
    {
        glUniform3f(dirLightLoc.Color, Light.Color.x, Light.Color.y, Light.Color.z);
        glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
        Vector3f LocalDirection = Light.GetLocalDirection();
        glUniform3f(dirLightLoc.Direction, LocalDirection.x, LocalDirection.y, LocalDirection.z);
        glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);
    }
    */


    void sendPointLight(unsigned int NumLights, Shader shader)
    {
        shader.setInteger("gNumPointLights", NumLights);

        for (unsigned int i = 0 ; i < NumLights ; i++) {
            shader.setVector3f(PointLightsName[i].Color, pointLights[i].Color.x, pointLights[i].Color.y, pointLights[i].Color.z);
            shader.setFloat(PointLightsName[i].AmbientIntensity, pointLights[i].AmbientIntensity);
            shader.setFloat(PointLightsName[i].DiffuseIntensity, pointLights[i].DiffuseIntensity);
            const glm::vec3& LocalPos = pointLights[i].GetLocalPosition();
            //LocalPos.Print();printf("\n");
            shader.setVector3f(PointLightsName[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
            shader.setFloat(PointLightsName[i].Atten.Constant, pointLights[i].Attenuation.Constant);
            shader.setFloat(PointLightsName[i].Atten.Linear, pointLights[i].Attenuation.Linear);
            shader.setFloat(PointLightsName[i].Atten.Exp, pointLights[i].Attenuation.Exp);
        }
    }

    void sendSpotLight(unsigned int NumLights, Shader shader)
    {
        shader.setInteger("gNumSpotLights", NumLights);

        for (unsigned int i = 0 ; i < NumLights ; i++) {
            shader.setVector3f(SpotLightsName[i].Color, spotLights[i].Color.x, spotLights[i].Color.y, spotLights[i].Color.z);
            shader.setFloat(SpotLightsName[i].AmbientIntensity, spotLights[i].AmbientIntensity);
            shader.setFloat(SpotLightsName[i].DiffuseIntensity, spotLights[i].DiffuseIntensity);
            const glm::vec3& LocalPos = spotLights[i].GetLocalPosition();
            shader.setVector3f(SpotLightsName[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
            glm::vec3 Direction = spotLights[i].GetLocalDirection();
            Direction = glm::normalize(Direction);
            shader.setVector3f(SpotLightsName[i].Direction, Direction.x, Direction.y, Direction.z);

            shader.setFloat(SpotLightsName[i].Cutoff, glm::cos(glm::radians(spotLights[i].Cutoff)));
            shader.setFloat(SpotLightsName[i].Atten.Constant, spotLights[i].Attenuation.Constant);
            shader.setFloat(SpotLightsName[i].Atten.Linear,   spotLights[i].Attenuation.Linear);
            shader.setFloat(SpotLightsName[i].Atten.Exp,      spotLights[i].Attenuation.Exp);
        }
    }


public:
    Lighting()
    {
        initPointLightsName(NUM_POINT_LIGHTS);
        initSpotLightsName(NUM_SPOT_LIGHTS);
    }


    void init()
    {
        pointLights[0].AmbientIntensity = 1.0f;
        pointLights[0].DiffuseIntensity = 1.0f;
        pointLights[0].Color = glm::vec3(1.0f, 1.0f, 0.0f);
        pointLights[0].Attenuation.Linear = 0.0f;
        pointLights[0].Attenuation.Exp = 0.0f;

        pointLights[1].DiffuseIntensity = 0.0f;
        pointLights[1].Color = glm::vec3(0.0f, 1.0f, 1.0f);
        pointLights[1].Attenuation.Linear = 0.0f;
        pointLights[1].Attenuation.Exp = 0.2f;

        spotLights[0].DiffuseIntensity = 1.0f;
        spotLights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
        spotLights[0].Attenuation.Linear = 0.01f;
        spotLights[0].Cutoff = 20.0f;

        spotLights[1].DiffuseIntensity = 1.0f;
        spotLights[1].Color = glm::vec3(1.0f, 1.0f, 0.0f);
        spotLights[1].Attenuation.Linear = 0.01f;
        spotLights[1].Cutoff = 30.0f;
    }


    void render(Shader shader, const WorldTrans& worldTransform, glm::vec3 cameraPos, glm::vec3 cameraTarget){
        pointLights[0].WorldPosition.x = 0.0f;
        pointLights[0].WorldPosition.y = 1.0;
        pointLights[0].WorldPosition.z = 1.0f;
        pointLights[0].CalcLocalPosition(worldTransform);

        pointLights[1].WorldPosition.x = 10.0f;
        pointLights[1].WorldPosition.y = 1.0f;
        pointLights[1].WorldPosition.z = 0.0f;
        pointLights[1].CalcLocalPosition(worldTransform);

        sendPointLight(2, shader);

        spotLights[0].WorldPosition = cameraPos;
        spotLights[0].WorldDirection = cameraTarget;
        spotLights[0].CalcLocalDirectionAndPosition(worldTransform);

        spotLights[1].WorldPosition = glm::vec3(0.0f, 1.0f, 0.0f);
        spotLights[1].WorldDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        spotLights[1].CalcLocalDirectionAndPosition(worldTransform);

        sendSpotLight(2, shader);
    }


};



#endif