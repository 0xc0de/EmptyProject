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

#include "Module.h"
#include "Player.h"
#include "Checker.h"

#include <World/Public/World.h>
#include <World/Public/Components/InputComponent.h>
#include <World/Public/Actors/PointLight.h>
#include <World/Public/Canvas.h>
#include <World/Public/Base/ResourceManager.h>
#include <World/Public/Widgets/WViewport.h>
#include <World/Public/MaterialGraph/MaterialGraph.h>

#include <Core/Public/Image.h>
#include <Runtime/Public/Runtime.h>

AN_CLASS_META( AModule )

AModule * GModule;

void AModule::OnGameStart() {

    GModule = this;

    SetInputMappings();
    CreateResources();

    World = AWorld::CreateWorld();

    ALevel * level = NewObject< ALevel >();
    World->AddLevel( level );
    //ALevel * level = World->GetPersistentLevel();

    RenderingParams = NewObject< ARenderingParameters >();
    RenderingParams->BackgroundColor = AColor4::Black();
    RenderingParams->bClearBackground = true;
    RenderingParams->bWireframe = false;
    RenderingParams->bDrawDebug = true;

    STransform spawnTransform;
    spawnTransform.Position = Float3(0.0f);
    spawnTransform.Rotation = Quat::Identity();
    spawnTransform.Scale = Float3( 1.0f ); //Float3(32, 0.1f, 32);

    AChecker * checker = World->SpawnActor< AChecker >( spawnTransform, level );
    AN_UNUSED( checker );

    spawnTransform.Position = Float3(0,3,0);
    spawnTransform.Rotation = Quat::Identity();
    spawnTransform.Scale = Float3(1.0f);
    APointLight * light = World->SpawnActor< APointLight >( spawnTransform, level );
    light->LightComponent->SetRadius( 10.0f );

    APlayer * player = World->SpawnActor< APlayer >( Float3(0,1.0f,2.0f), Quat::Identity(), level );

    // Spawn player controller
    PlayerController = World->SpawnActor< APlayerController >();
    PlayerController->SetPlayerIndex( CONTROLLER_PLAYER_1 );
    PlayerController->SetInputMappings( InputMappings );
    PlayerController->SetRenderingParameters( RenderingParams );
    //PlayerController->SetHUD( hud );
    PlayerController->GetInputComponent()->MouseSensitivity = 0.3f;
    PlayerController->SetPawn( player );

    WDesktop * desktop = NewObject< WDesktop >();
    GEngine.SetDesktop( desktop );

    desktop->AddWidget(
        &WWidget::New< WViewport >()
        .SetPlayerController( PlayerController )
        .SetHorizontalAlignment( WIDGET_ALIGNMENT_STRETCH )
        .SetVerticalAlignment( WIDGET_ALIGNMENT_STRETCH )
        .SetFocus()
    );
}

void AModule::OnGameEnd() {
}

void AModule::SetInputMappings() {
    InputMappings = NewObject< AInputMappings >();

    InputMappings->MapAxis( "MoveForward", ID_KEYBOARD, KEY_W, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "MoveForward", ID_KEYBOARD, KEY_S, -1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "MoveRight", ID_KEYBOARD, KEY_A, -1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "MoveRight", ID_KEYBOARD, KEY_D, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "MoveUp", ID_KEYBOARD, KEY_SPACE, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "MoveDown", ID_KEYBOARD, KEY_C, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "TurnRight", ID_MOUSE, MOUSE_AXIS_X, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAxis( "TurnUp", ID_MOUSE, MOUSE_AXIS_Y, 1.0f, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "Speed", ID_KEYBOARD, KEY_LEFT_SHIFT, 0, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "Pause", ID_KEYBOARD, KEY_P, 0, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "Pause", ID_KEYBOARD, KEY_PAUSE, 0, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "TakeScreenshot", ID_KEYBOARD, KEY_F12, 0, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "ToggleWireframe", ID_KEYBOARD, KEY_Y, 0, CONTROLLER_PLAYER_1 );
    InputMappings->MapAction( "ToggleDebugDraw", ID_KEYBOARD, KEY_G, 0, CONTROLLER_PLAYER_1 );
}

void AModule::CreateResources() {
#if 0
    

    

    

    // Skybox texture
    {
        const char * Cubemap[6] = {
            "DarkSky/rt.tga",
            "DarkSky/lt.tga",
            "DarkSky/up.tga",
            "DarkSky/dn.tga",
            "DarkSky/bk.tga",
            "DarkSky/ft.tga"
        };
        AImage rt, lt, up, dn, bk, ft;
        AImage const * cubeFaces[6] = { &rt,&lt,&up,&dn,&bk,&ft };
        rt.Load( Cubemap[0], nullptr, IMAGE_PF_BGR16F );
        lt.Load( Cubemap[1], nullptr, IMAGE_PF_BGR16F );
        up.Load( Cubemap[2], nullptr, IMAGE_PF_BGR16F );
        dn.Load( Cubemap[3], nullptr, IMAGE_PF_BGR16F );
        bk.Load( Cubemap[4], nullptr, IMAGE_PF_BGR16F );
        ft.Load( Cubemap[5], nullptr, IMAGE_PF_BGR16F );
        //TODO: convert to 16F
        //const float HDRI_Scale = 4.0f;
        //const float HDRI_Pow = 1.1f;
        //for ( int i = 0 ; i < 6 ; i++ ) {
        //    float * HDRI = (float*)cubeFaces[i]->pRawData;
        //    int count = cubeFaces[i]->Width*cubeFaces[i]->Height*3;
        //    for ( int j = 0; j < count ; j += 3 ) {
        //        HDRI[j] = StdPow( HDRI[j + 0] * HDRI_Scale, HDRI_Pow );
        //        HDRI[j + 1] = StdPow( HDRI[j + 1] * HDRI_Scale, HDRI_Pow );
        //        HDRI[j + 2] = StdPow( HDRI[j + 2] * HDRI_Scale, HDRI_Pow );
        //    }
        //}
        ATexture * SkyboxTexture = NewObject< ATexture >();
        SkyboxTexture->InitializeCubemapFromImages( cubeFaces );
        RegisterResource( SkyboxTexture, "SkyboxTexture" );
    }

    
#endif

    

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
        //static TStaticResourceFinder< ATexture > SkyboxTexture( _CTS( "SkyboxTexture" ) );
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

        MGInTexCoord * inTexCoordBlock = graph->AddNode< MGInTexCoord >();

        MGVertexStage * materialVertexStage = graph->AddNode< MGVertexStage >();

        MGNextStageVariable * texCoord = materialVertexStage->AddNextStageVariable( "TexCoord", AT_Float2 );
        texCoord->Connect( inTexCoordBlock, "Value" );

        MGTextureSlot * diffuseTexture = graph->AddNode< MGTextureSlot >();
        diffuseTexture->SamplerDesc.Filter = TEXTURE_FILTER_MIPMAP_TRILINEAR;

        MGSampler * textureSampler = graph->AddNode< MGSampler >();
        textureSampler->TexCoord->Connect( materialVertexStage, "TexCoord" );
        textureSampler->TextureSlot->Connect( diffuseTexture, "Value" );

        MGFragmentStage * materialFragmentStage = graph->AddNode< MGFragmentStage >();
        materialFragmentStage->Color->Connect( textureSampler, "RGBA" );

        graph->VertexStage = materialVertexStage;
        graph->FragmentStage = materialFragmentStage;
        graph->MaterialType = MATERIAL_TYPE_PBR;
        graph->RegisterTextureSlot( diffuseTexture );

        AMaterialBuilder * builder = NewObject< AMaterialBuilder >();
        builder->Graph = graph;

        SMaterialDef def;
        builder->BuildData( def );

        AMaterial * material = NewObject< AMaterial >();
        material->Initialize( &def );
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
        mesh->InitializePlaneMesh( 256, 256, 256 );
        ACollisionBox * box = mesh->BodyComposition.AddCollisionBody< ACollisionBox >();
        box->HalfExtents.X = 128;
        box->HalfExtents.Y = 0.1f;
        box->HalfExtents.Z = 128;
        box->Position.Y -= box->HalfExtents.Y;
        mesh->SetMaterialInstance( 0, MaterialInst.GetObject() );
        RegisterResource( mesh, "DefaultShapePlane256x256x256" );
    }

    //// Checker mesh
    //{
    //    static TStaticResourceFinder< AMaterialInstance > MaterialInst( _CTS( "Grid8MaterialInstance" ) );
    //    AIndexedMesh * CheckerMesh = NewObject< AIndexedMesh >();
    //    CheckerMesh->InitializeFromFile( "/Default/Meshes/Box" );
    //    CheckerMesh->SetMaterialInstance( 0, MaterialInst.GetObject() );
    //    RegisterResource( CheckerMesh, "CheckerMesh" );
    //}
}

#include <Runtime/Public/EntryDecl.h>

static SEntryDecl ModuleDecl = {
    // Game title
    "AngieEngine: Empty Project",
    // Root path
    "Data",
    // Module class
    &AModule::ClassMeta()
};

AN_ENTRY_DECL( ModuleDecl )
