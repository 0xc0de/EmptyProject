/*

Angie Engine Source Code

MIT License

Copyright (C) 2017-2020 Alexander Samusev.

This file is part of the Angie Engine Source Code.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <World/Public/World.h>
#include <World/Public/Base/GameModuleInterface.h>
#include <World/Public/Base/ResourceManager.h>
#include <World/Public/Components/InputComponent.h>
#include <World/Public/Components/MeshComponent.h>
#include <World/Public/Actors/PointLight.h>
#include <World/Public/Actors/Pawn.h>
#include <World/Public/Actors/PlayerController.h>
#include <World/Public/Widgets/WViewport.h>
#include <World/Public/MaterialGraph/MaterialGraph.h>
#include <GameThread/Public/EngineInstance.h>

#include <Core/Public/Image.h>
#include <Runtime/Public/Runtime.h>

class AGround : public APawn
{
    AN_ACTOR( AGround, APawn )

protected:
    AGround()
    {
        static TStaticResourceFinder< AIndexedMesh > CheckerMesh( _CTS( "DefaultShapePlane256x256x256" ) );

        AMeshComponent * component = CreateComponent< AMeshComponent >( "Ground" );
        component->SetMesh( CheckerMesh.GetObject() );
        component->CopyMaterialsFromMeshResource();

        RootComponent = component;

        bCanEverTick = false;
    }
};
AN_CLASS_META( AGround )

class APlayer : public APawn
{
    AN_ACTOR( APlayer, APawn )

public:
    ACameraComponent * Camera;

protected:
    APlayer()
    {
        static TStaticResourceFinder< AIndexedMesh > CheckerMesh( _CTS( "CheckerMesh" ) );

        Camera = CreateComponent< ACameraComponent >( "Camera" );

        RootComponent = Camera;
        PawnCamera = Camera;

        bCanEverTick = true;

        static TStaticResourceFinder< AIndexedMesh > SkyboxMesh( _CTS( "/Default/Meshes/Skybox" ) );
        static TStaticResourceFinder< AMaterialInstance > SkyboxMaterialInst( _CTS( "SkyboxMaterialInstance" ) );
        Skybox = CreateComponent< AMeshComponent >( "Skybox" );
        Skybox->SetMotionBehavior( MB_KINEMATIC );
        Skybox->SetMesh( SkyboxMesh.GetObject() );
        Skybox->SetMaterialInstance( SkyboxMaterialInst.GetObject() );
        Skybox->ForceOutdoor( true );
        Skybox->AttachTo( Camera );
        Skybox->SetAbsoluteRotation( true );
        Skybox->SetVisibilityGroup( VISIBILITY_GROUP_SKYBOX );
    }

    float CalcYaw( Float3 const & RightVec, Float3 const & BackVec )
    {
        Float2 projected( BackVec.X, BackVec.Z );
        float lenSqr = projected.LengthSqr();
        if ( lenSqr < 0.0001 ) {
            projected.X = RightVec.X;
            projected.Y = RightVec.Z;
            projected.NormalizeSelf();
            return Math::Degrees( Math::Atan2( projected.X, projected.Y ) ) + 90;
        }
        projected.NormalizeSelf();
        return Math::Degrees( Math::Atan2( projected.X, projected.Y ) );
    }

    void BeginPlay() override
    {
        Super::BeginPlay();

        Angles.Yaw = CalcYaw( RootComponent->GetRightVector(), RootComponent->GetBackVector() );
        Angles.Pitch = Angles.Roll = 0;

        RootComponent->SetAngles( Angles );
    }

    void SetupPlayerInputComponent( AInputComponent * _Input ) override
    {
        _Input->BindAxis( "MoveForward", this, &APlayer::MoveForward );
        _Input->BindAxis( "MoveRight", this, &APlayer::MoveRight );
        _Input->BindAxis( "MoveUp", this, &APlayer::MoveUp );
        _Input->BindAxis( "MoveDown", this, &APlayer::MoveDown );
        _Input->BindAxis( "TurnRight", this, &APlayer::TurnRight );
        _Input->BindAxis( "TurnUp", this, &APlayer::TurnUp );
        _Input->BindAction( "Speed", IA_PRESS, this, &APlayer::SpeedPress );
        _Input->BindAction( "Speed", IA_RELEASE, this, &APlayer::SpeedRelease );
    }

    void Tick( float _TimeStep ) override
    {
        Super::Tick( _TimeStep );

        constexpr float PLAYER_MOVE_SPEED = 1.5f; // Meters per second
        constexpr float PLAYER_MOVE_HIGH_SPEED = 3.0f;

        const float MoveSpeed = _TimeStep * (bSpeed ? PLAYER_MOVE_HIGH_SPEED : PLAYER_MOVE_SPEED);
        float lenSqr = MoveVector.LengthSqr();
        if ( lenSqr > 0 ) {
            RootComponent->Step( MoveVector.Normalized() * MoveSpeed );
            MoveVector.Clear();
        }
    }

    void DrawDebug( ADebugRenderer * InRenderer )
    {
        Super::DrawDebug( InRenderer );
    }

private:
    void MoveForward( float _Value )
    {
        MoveVector += RootComponent->GetForwardVector() * Math::Sign( _Value );
    }

    void MoveRight( float _Value )
    {
        MoveVector += RootComponent->GetRightVector() * Math::Sign( _Value );
    }

    void MoveUp( float _Value )
    {
        MoveVector.Y += 1;
    }

    void MoveDown( float _Value )
    {
        MoveVector.Y -= 1;
    }

    void TurnRight( float _Value )
    {
        Angles.Yaw -= _Value;
        Angles.Yaw = Angl::Normalize180( Angles.Yaw );
        RootComponent->SetAngles( Angles );
    }

    void TurnUp( float _Value )
    {
        Angles.Pitch += _Value;
        Angles.Pitch = Math::Clamp( Angles.Pitch, -90.0f, 90.0f );
        RootComponent->SetAngles( Angles );
    }

    void SpeedPress()
    {
        bSpeed = true;
    }

    void SpeedRelease()
    {
        bSpeed = false;
    }

    AMeshComponent * Skybox;
    Angl Angles;
    Float3 MoveVector;
    bool bSpeed;
};
AN_CLASS_META( APlayer )

class AModule final : public IGameModule
{
    AN_CLASS( AModule, IGameModule )

public:
    AModule()
    {
        CreateResources();

        AInputMappings * inputMappings = NewObject< AInputMappings >();

        inputMappings->MapAxis( "MoveForward", ID_KEYBOARD, KEY_W, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "MoveForward", ID_KEYBOARD, KEY_S, -1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "MoveRight", ID_KEYBOARD, KEY_A, -1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "MoveRight", ID_KEYBOARD, KEY_D, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "MoveUp", ID_KEYBOARD, KEY_SPACE, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "MoveDown", ID_KEYBOARD, KEY_C, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "TurnRight", ID_MOUSE, MOUSE_AXIS_X, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAxis( "TurnUp", ID_MOUSE, MOUSE_AXIS_Y, 1.0f, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "Speed", ID_KEYBOARD, KEY_LEFT_SHIFT, 0, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "Pause", ID_KEYBOARD, KEY_P, 0, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "Pause", ID_KEYBOARD, KEY_PAUSE, 0, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "TakeScreenshot", ID_KEYBOARD, KEY_F12, 0, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "ToggleWireframe", ID_KEYBOARD, KEY_Y, 0, CONTROLLER_PLAYER_1 );
        inputMappings->MapAction( "ToggleDebugDraw", ID_KEYBOARD, KEY_G, 0, CONTROLLER_PLAYER_1 );

        AWorld * world = AWorld::CreateWorld();

        ALevel * level = NewObject< ALevel >();
        world->AddLevel( level );

        ARenderingParameters * renderingParams = NewObject< ARenderingParameters >();
        renderingParams->BackgroundColor = AColor4::Black();
        renderingParams->bClearBackground = true;
        renderingParams->bWireframe = false;
        renderingParams->bDrawDebug = true;

        STransform spawnTransform;

        spawnTransform.Position = Float3( 0.0f );
        spawnTransform.Rotation = Quat::Identity();
        spawnTransform.Scale = Float3( 1.0f );
        world->SpawnActor< AGround >( spawnTransform, level );

        spawnTransform.Position = Float3( 0, 3, 0 );
        spawnTransform.Rotation = Quat::Identity();
        spawnTransform.Scale = Float3( 1.0f );
        APointLight * light = world->SpawnActor< APointLight >( spawnTransform, level );
        light->LightComponent->SetRadius( 10.0f );

        APlayer * player = world->SpawnActor< APlayer >( Float3( 0, 1.0f, 2.0f ), Quat::Identity(), level );

        // Spawn player controller
        APlayerController * playerController = world->SpawnActor< APlayerController >();
        playerController->SetPlayerIndex( CONTROLLER_PLAYER_1 );
        playerController->SetInputMappings( inputMappings );
        playerController->SetRenderingParameters( renderingParams );
        playerController->GetInputComponent()->MouseSensitivity = 0.3f;
        playerController->SetPawn( player );

        WDesktop * desktop = NewObject< WDesktop >();
        GEngine.SetDesktop( desktop );

        desktop->AddWidget(
            &WNew( WViewport )
            .SetPlayerController( playerController )
            .SetHorizontalAlignment( WIDGET_ALIGNMENT_STRETCH )
            .SetVerticalAlignment( WIDGET_ALIGNMENT_STRETCH )
            .SetFocus()
        );
    }

private:
    void CreateResources()
    {
        // Skybox texture
        {
            byte data[1] = {};

            ATexture * texture = NewObject< ATexture >();

            texture->InitializeCubemap( TEXTURE_PF_R8_UNORM, 1, 1 );

            for ( int face = 0 ; face < 6 ; face++ ) {
                texture->WriteTextureDataCubemap( 0, 0, 1, 1, face, 0, data );
            }

            RegisterResource( texture, "BlackSky" );
        }

        // Skybox material instance
        {
            static TStaticResourceFinder< AMaterial > SkyboxMaterial( _CTS( "/Default/Materials/Skybox" ) );
            static TStaticResourceFinder< ATexture > SkyboxTexture( _CTS( "BlackSky" ) );
            AMaterialInstance * SkyboxMaterialInstance = NewObject< AMaterialInstance >();
            SkyboxMaterialInstance->SetMaterial( SkyboxMaterial.GetObject() );
            SkyboxMaterialInstance->SetTexture( 0, SkyboxTexture.GetObject() );
            RegisterResource( SkyboxMaterialInstance, "SkyboxMaterialInstance" );
        }

        // Texture Grid8
        {
            GetOrCreateResource< ATexture >( "Grid8", "/Common/grid8.png" );
        }

        // Material
        {
            MGMaterialGraph * graph = NewObject< MGMaterialGraph >();

            MGInTexCoord * inTexCoord = graph->AddNode< MGInTexCoord >();

            MGTextureSlot * diffuseTexture = graph->AddNode< MGTextureSlot >();
            diffuseTexture->SamplerDesc.Filter = TEXTURE_FILTER_MIPMAP_TRILINEAR;

            MGSampler * textureSampler = graph->AddNode< MGSampler >();
            textureSampler->TexCoord->Connect( inTexCoord, "Value" );
            textureSampler->TextureSlot->Connect( diffuseTexture, "Value" );

            graph->Color->Connect( textureSampler, "RGBA" );

            graph->MaterialType = MATERIAL_TYPE_PBR;
            graph->RegisterTextureSlot( diffuseTexture );

            AMaterial * material = NewObject< AMaterial >();
            material->Initialize( graph );
            RegisterResource( material, "MyMaterial" );
        }

        // CheckerMaterialInstance
        {
            static TStaticResourceFinder< AMaterial > MaterialResource( _CTS( "MyMaterial" ) );
            static TStaticResourceFinder< ATexture > TextureResource( _CTS( "Grid8" ) );
            AMaterialInstance * CheckerMaterialInstance = NewObject< AMaterialInstance >();
            CheckerMaterialInstance->SetMaterial( MaterialResource.GetObject() );
            CheckerMaterialInstance->SetTexture( 0, TextureResource.GetObject() );
            RegisterResource( CheckerMaterialInstance, "Grid8MaterialInstance" );
        }

        //
        // Example, how to create mesh resource and register it
        //
        {
            static TStaticResourceFinder< AMaterialInstance > MaterialInst( _CTS( "Grid8MaterialInstance" ) );

            AIndexedMesh * mesh = NewObject< AIndexedMesh >();
            mesh->InitializePlaneMeshXZ( 256, 256, 256 );
            ACollisionBox * box = mesh->BodyComposition.AddCollisionBody< ACollisionBox >();
            box->HalfExtents.X = 128;
            box->HalfExtents.Y = 0.1f;
            box->HalfExtents.Z = 128;
            box->Position.Y -= box->HalfExtents.Y;
            mesh->SetMaterialInstance( 0, MaterialInst.GetObject() );
            RegisterResource( mesh, "DefaultShapePlane256x256x256" );
        }
    }
};
AN_CLASS_META( AModule )

#include <Runtime/Public/EntryDecl.h>

static SEntryDecl ModuleDecl = {
    // Game title
    "Angie Engine: Empty Project",
    // Root path
    "Data",
    // Module class
    &AModule::ClassMeta()
};

AN_ENTRY_DECL( ModuleDecl )
