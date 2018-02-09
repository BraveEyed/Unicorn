/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/UnicornRender.hpp>
#include <unicorn/video/Graphics.hpp>
#include <unicorn/system/Window.hpp>
#include <unicorn/system/WindowHint.hpp>
#include <unicorn/system/CustomValue.hpp>
#include <unicorn/Settings.hpp>
#include <unicorn/system/Input.hpp>
#include <unicorn/system/input/Action.hpp>
#include <unicorn/system/input/Key.hpp>
#include <unicorn/system/input/Modifier.hpp>
#include <unicorn/video/Renderer.hpp>
#include <unicorn/video/Primitives.hpp>
#include <unicorn/video/Texture.hpp>
#include <unicorn/video/Material.hpp>

#include <unicorn/video/Camera.hpp>
#include <unicorn/video/Camera2DController.hpp>
#include <unicorn/video/CameraFpsController.hpp>
#include <unicorn/video/OrthographicCamera.hpp>
#include <unicorn/video/PerspectiveCamera.hpp>

#include <glm/glm.hpp>

#include <ctime>
#include <array>
#include <cstdlib>
#include <iostream>
#include <list>
#include <memory>

static unicorn::video::Graphics* pGraphics = nullptr;
static unicorn::system::Timer* timer = nullptr;
static unicorn::video::Renderer* vkRenderer = nullptr;
static unicorn::system::Input* pInput = nullptr;
static bool depthTest = true;
unicorn::system::Window* pWindow0 = nullptr;
std::list<unicorn::video::Mesh*> meshes;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

const float xSens = 0.05f;
const float ySens = 0.05f;

//Camera settings
bool isPerspective = true;
unicorn::video::Camera* perspective = nullptr;
unicorn::video::Camera* ortho = nullptr;
unicorn::video::CameraFpsController* pCameraFpsController = nullptr;
unicorn::video::Camera2DController* pCamera2DController = nullptr;
unicorn::video::PerspectiveCamera* pPerspectiveProjection = nullptr;
unicorn::video::OrthographicCamera* pOrthoProjection = nullptr;

void onLogicFrame(unicorn::UnicornRender* /*render*/)
{
    float const currentFrame = static_cast<float>(timer->ElapsedMilliseconds().count()) / 1000;
    float const newDeltatime = currentFrame - lastFrame;

    if(newDeltatime <= 0.0)
    {
        return;
    }

    deltaTime = newDeltatime;

    // Updating transformations for meshes
    for(auto& mesh : meshes)
    {
        mesh->UpdateTransformMatrix();
    }

    // Updating transformations for cameras
    pCameraFpsController->Update();
    pCamera2DController->Update();

    lastFrame = currentFrame;
}

void onMouseButton(unicorn::system::Window::MouseButtonEvent const& mouseButtonEvent)
{
    using unicorn::system::input::MouseButton;
    using unicorn::system::input::Action;

    unicorn::system::input::Action const& action = mouseButtonEvent.action;

    if(action == Action::Release || action == Action::Repeat)
    {
        return;
    }

    unicorn::system::input::MouseButton const& button = mouseButtonEvent.button;

    switch(button)
    {
        case MouseButton::MouseLeft:
        {
            auto cubematerial = std::make_shared<unicorn::video::Material>();
            glm::vec3 const color = { static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255 };
            cubematerial->SetColor(color);

            unicorn::video::Mesh* mesh = new unicorn::video::Mesh;
            unicorn::video::Primitives::Sphere(*mesh, 1, 32, 32);
            mesh->SetMaterial(cubematerial);
            glm::vec3 const randomTranslate = { std::rand() % 40 - 20, std::rand() % 40 - 20, std::rand() % 40 - 20 };
            mesh->SetTranslation(mesh->GetTranslation() + randomTranslate);

            vkRenderer->AddMesh(mesh);
            meshes.push_back(mesh);
            break;
        }
        case MouseButton::MouseMiddle:
        {
            if(!meshes.empty())
            {
                // Get random cube
                auto meshIt = meshes.begin();

                std::advance(meshIt, std::rand() % meshes.size());

                auto mesh = *meshIt;

                auto cubeMaterial = std::make_shared<unicorn::video::Material>();
                cubeMaterial->SetColor({ static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255 });
                cubeMaterial->SetIsWired(true);
                mesh->SetMaterial(cubeMaterial);
            }
            break;
        }
        case MouseButton::MouseRight:
        {
            if(!meshes.empty())
            {
                // Get random cube
                auto meshIt = meshes.begin();

                std::advance(meshIt, std::rand() % meshes.size());

                auto const mesh = *meshIt;

                // Release cube's mesh
                vkRenderer->DeleteMesh(mesh);

                //// Erase cube
                meshes.erase(meshIt);
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void onCursorPositionChanged(unicorn::system::Window* pWindow, std::pair<double, double> pos)
{
    const float xOffset = static_cast<float>(pos.first) * xSens;
    const float yOffset = static_cast<float>(pos.second) * ySens;
    if (isPerspective)
    {
        pCameraFpsController->UpdateView(xOffset, yOffset);
    }
    else
    {
        pCameraFpsController->SetViewPositions(xOffset, yOffset);
    }
}

void onMouseScrolled(unicorn::system::Window* pWindow, std::pair<double, double> pos)
{
    if (isPerspective)
    {
        pPerspectiveProjection->SetFov(pPerspectiveProjection->GetFov() -
            static_cast<float>(pos.second / 50)); // 50 is zoom coefficient
    }
    else
    {
        pOrthoProjection->SetScale(pOrthoProjection->GetScale() +
            static_cast<float>(pos.second * 10)); // 10 is scale speed
    }
}

void onWindowKeyboard(unicorn::system::Window::KeyboardEvent const& keyboardEvent)
{
    using unicorn::system::input::Key;
    using unicorn::system::input::Modifier;
    using unicorn::system::input::Action;
    using unicorn::system::MouseMode;

    unicorn::system::input::Action const& action = keyboardEvent.action;

    if(Action::Release == action)
    {
        return;
    }

    unicorn::system::Window* const& pWindow = keyboardEvent.pWindow;
    auto const& key = keyboardEvent.key;
    auto const& modifiers = keyboardEvent.modifiers;

    float const time = deltaTime * 0.1f;
    float speed = 50.f;

    if(Modifier::Shift & modifiers)
    {
        speed *= 10;
    }

    if(Modifier::Alt & modifiers)
    {
        speed *= 5;
    }

    switch(key)
    {
        case Key::Normal_1:
        {
            isPerspective = true;
            vkRenderer->camera = perspective;
            break;
        }
        case Key::Normal_2:
        {
            isPerspective = false;
            vkRenderer->camera = ortho;
            break;
        }
        case Key::W:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(0, 0, time * speed));
            }
            else
            {
                pCamera2DController->TranslateLocal(glm::vec3(0, time * speed, 0));
            }
            break;
        }
        case Key::S:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(0, 0, time * -speed));
            }
            else
            {
                pCamera2DController->TranslateLocal(glm::vec3(0, time * -speed, 0));
            }
            break;
        }
        case Key::A:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(time * -speed, 0, 0));
            }
            else
            {
                pCamera2DController->TranslateLocal(glm::vec3(time * -speed, 0, 0));
            }
            break;
        }
        case Key::D:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(time * speed, 0, 0));
            }
            else
            {
                pCamera2DController->TranslateLocal(glm::vec3(time * speed, 0, 0));
            }
            break;
        }
        case Key::Space:
            {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(0, time * speed, 0));
            }
            break;
        }
        case Key::LeftControl:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocal(glm::vec3(0, time * -speed, 0));
            }
            break;
        }
        case Key::N:
        {
            if (isPerspective)
            {
                pCameraFpsController->Rotate(static_cast<float>(glm::radians(1.f)), glm::vec3(0., 1., 0.));
            }
            break;
        }
        case Key::Q:
        {
            if (isPerspective)
            {
                pCameraFpsController->Rotate(static_cast<float>(-glm::radians(1.)), glm::vec3(0., 0., 1.));
            }
            break;
        }
        case Key::E:
        {
            if (isPerspective)
            {
                pCameraFpsController->Rotate(static_cast<float>(glm::radians(1.)), glm::vec3(0., 0., 1.));
            }
            break;
        }
        case Key::Up:
        {
            pCameraFpsController->Rotate(static_cast<float>(glm::radians(-1.)), glm::vec3(1., 0., 0.));
            break;
        }
        case Key::Down:
        {
            pCameraFpsController->Rotate(static_cast<float>(glm::radians(1.)), glm::vec3(1., 0., 0.));
            break;
        }
        case Key::Left:
        {
            pCameraFpsController->Rotate(static_cast<float>(glm::radians(1.)), glm::vec3(0., 1., 0.));
            break;
        }
        case Key::Right:
        {
            pCameraFpsController->Rotate(static_cast<float>(glm::radians(-1.)), glm::vec3(0., 1., 0.));
            break;
        }
        case Key::C:
        {
            pWindow->SetMouseMode(MouseMode::Captured);
            break;
        }
        case Key::V:
        {
            if(Action::Repeat == action)
            {
                return;
            }
            depthTest = !depthTest;
            pGraphics->SetDepthTest(depthTest);
            break;
        }
        case Key::Escape:
        {
            pWindow->SetMouseMode(MouseMode::Normal);
            break;
        }
        case Key::Insert:
        {
            if (pInput)
            {
                switch(modifiers)
                {
                    case Modifier::Ctrl:
                    {
                        pInput->SetClipboard(std::string("Gotta go fast"));
                        break;
                    }
                    case Modifier::Shift:
                    {
                        std::cout << "Clipboard data: " << pInput->GetClipboard() << std::endl;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void onRendererDestroyed(unicorn::video::Renderer* pRenderer)
{
    if(vkRenderer == pRenderer)
    {
        vkRenderer = nullptr;
    }
}

std::list<unicorn::video::Mesh*> MakeCubeMap()
{
    std::list<unicorn::video::Mesh*> cubemapMeshes;

    auto frontSkyBox = std::make_shared<unicorn::video::Texture>();
    auto backSkyBox = std::make_shared<unicorn::video::Texture>();
    auto leftSkyBox = std::make_shared<unicorn::video::Texture>();
    auto rightSkyBox = std::make_shared<unicorn::video::Texture>();
    auto topSkyBox = std::make_shared<unicorn::video::Texture>();
    auto bottomSkyBox = std::make_shared<unicorn::video::Texture>();

    frontSkyBox->Load("data/textures/cubemap/posz.jpg");
    backSkyBox->Load("data/textures/cubemap/negz.jpg");
    leftSkyBox->Load("data/textures/cubemap/negx.jpg");
    rightSkyBox->Load("data/textures/cubemap/posx.jpg");
    topSkyBox->Load("data/textures/cubemap/posy.jpg");
    bottomSkyBox->Load("data/textures/cubemap/negy.jpg");


    //Checks data
    if (!frontSkyBox->IsLoaded()
        || !backSkyBox->IsLoaded()
        || !leftSkyBox->IsLoaded()
        || !rightSkyBox->IsLoaded()
        || !topSkyBox->IsLoaded()
        || !bottomSkyBox->IsLoaded())
    {
        return cubemapMeshes;
    }

    auto frontTextureMat = std::make_shared<unicorn::video::Material>();
    auto backTextureMat = std::make_shared<unicorn::video::Material>();
    auto leftTextureMat = std::make_shared<unicorn::video::Material>();
    auto rightTextureMat = std::make_shared<unicorn::video::Material>();
    auto upTextureMat = std::make_shared<unicorn::video::Material>();
    auto bottomTextureMat = std::make_shared<unicorn::video::Material>();

    frontTextureMat->SetAlbedo(frontSkyBox);
    backTextureMat->SetAlbedo(backSkyBox);
    leftTextureMat->SetAlbedo(leftSkyBox);
    rightTextureMat->SetAlbedo(rightSkyBox);
    upTextureMat->SetAlbedo(topSkyBox);
    bottomTextureMat->SetAlbedo(bottomSkyBox);

    using unicorn::video::Primitives;

    unicorn::video::Mesh* frontBox = new unicorn::video::Mesh;
    Primitives::Quad(*frontBox);
    unicorn::video::Mesh* backBox = new unicorn::video::Mesh;
    Primitives::Quad(*backBox);
    unicorn::video::Mesh* leftBox = new unicorn::video::Mesh;
    Primitives::Quad(*leftBox);
    unicorn::video::Mesh* rightBox = new unicorn::video::Mesh;
    Primitives::Quad(*rightBox);
    unicorn::video::Mesh* upBox = new unicorn::video::Mesh;
    Primitives::Quad(*upBox);
    unicorn::video::Mesh* bottomBox = new unicorn::video::Mesh;
    Primitives::Quad(*bottomBox);

    frontBox->SetMaterial(frontTextureMat);
    backBox->SetMaterial(backTextureMat);
    leftBox->SetMaterial(leftTextureMat);
    rightBox->SetMaterial(rightTextureMat);
    upBox->SetMaterial(upTextureMat);
    bottomBox->SetMaterial(bottomTextureMat);

    cubemapMeshes.push_back(frontBox);
    cubemapMeshes.push_back(backBox);
    cubemapMeshes.push_back(leftBox);
    cubemapMeshes.push_back(rightBox);
    cubemapMeshes.push_back(upBox);
    cubemapMeshes.push_back(bottomBox);

    //Skybox
    const float skyBoxScaleFactor = 500;
    const float skyBoxDistance = skyBoxScaleFactor / 2 - 1;

    frontBox->SetTranslation({ 0, 0, skyBoxDistance });
    backBox->SetTranslation({ 0, 0, -skyBoxDistance });
    backBox->Rotate(static_cast<float>(glm::radians(-180.0)), { 0.0, 1.0, 0.0 });

    upBox->SetTranslation({ 0, skyBoxDistance, 0 });
    upBox->Rotate(static_cast<float>(glm::radians(-90.0)), { 1.0, 0.0, 0.0 });
    bottomBox->SetTranslation({ 0, -skyBoxDistance, 0 });
    bottomBox->Rotate(static_cast<float>(glm::radians(90.0)), { 1.0, 0.0, 0.0 });

    leftBox->SetTranslation({ -skyBoxDistance, 0, 0 });
    leftBox->Rotate(static_cast<float>(glm::radians(-90.0)), { 0.0, 1.0, 0.0 });
    rightBox->SetTranslation({ skyBoxDistance, 0, 0 });
    rightBox->Rotate(static_cast<float>(glm::radians(90.0)), { 0.0, 1.0, 0.0 });

    frontBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    backBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    upBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    bottomBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    leftBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    rightBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });

    return cubemapMeshes;
}

int main(int argc, char* argv[])
{
    unicorn::Settings& settings = unicorn::Settings::Instance();

    settings.Init(argc, argv, "Sanic_Jymper.log");
    settings.SetApplicationName("SANIC JYMPER");
    auto* unicornRender = new unicorn::UnicornRender;
    timer = new unicorn::system::Timer(true);
    if (unicornRender->Init())
    {
        pGraphics = unicornRender->GetGraphics();
        pInput = unicornRender->GetInput();

        unicornRender->LogicFrame.connect(&onLogicFrame);

        pGraphics->SetWindowCreationHint(unicorn::system::WindowHint::Decorated,
                                         unicorn::system::CustomValue::True);

        pGraphics->SetWindowCreationHint(unicorn::system::WindowHint::Resizable,
                                         unicorn::system::CustomValue::True);

        auto const h = pGraphics->GetMonitors().back()->GetActiveVideoMode().height;
        auto const w = pGraphics->GetMonitors().back()->GetActiveVideoMode().width;
        settings.SetApplicationHeight(static_cast<uint32_t>(h));
        settings.SetApplicationWidth(static_cast<uint32_t>(w));

        auto pWindow0 = pGraphics->SpawnWindow(settings.GetApplicationWidth(),
           settings.GetApplicationHeight(),
           settings.GetApplicationName(),
           nullptr,
           nullptr);
        pWindow0->SetMouseMode(unicorn::system::MouseMode::Normal);

        perspective = new unicorn::video::Camera;
        ortho = new unicorn::video::Camera;

        vkRenderer = pGraphics->SpawnRenderer(pWindow0, *perspective);
        if(vkRenderer == nullptr)
        {
            return -1;
        }
        vkRenderer->Destroyed.connect(&onRendererDestroyed);

        // Configuring cameras
        pPerspectiveProjection = new unicorn::video::PerspectiveCamera(*pWindow0, perspective->projection);
        pOrthoProjection = new unicorn::video::OrthographicCamera(*pWindow0, ortho->projection);

        pCameraFpsController = new unicorn::video::CameraFpsController(perspective->view);
        pCamera2DController = new unicorn::video::Camera2DController(ortho->view);
        {
            using unicorn::video::Primitives;

            auto cubemap = MakeCubeMap();
            auto grassTexture = std::make_shared<unicorn::video::Texture>();
            grassTexture->Load("data/textures/grass.png");

            auto grassMaterial = std::make_shared<unicorn::video::Material>();
            grassMaterial->SetAlbedo(grassTexture);

            auto colorMaterial = std::make_shared<unicorn::video::Material>();
            colorMaterial->SetColor(unicorn::video::Color::LightPink());

            unicorn::video::Mesh* pinkBoxGeometry = new unicorn::video::Mesh;
            Primitives::Box(*pinkBoxGeometry);
            unicorn::video::Mesh* grassQuad = new unicorn::video::Mesh;
            Primitives::Quad(*grassQuad);
            grassQuad->SetMaterial(grassMaterial);

            pinkBoxGeometry->SetMaterial(colorMaterial);

            auto gltfModel = Primitives::LoadModel("data/models/glTF/DamagedHelmet.gltf");

            pCameraFpsController->TranslateLocal({ 0, 0, 1 });
            pCameraFpsController->SetOrientation({ 0, 0, -1 });
            grassQuad->TranslateLocal({ 0, 0, -4 });
            for (auto mesh : gltfModel)
            {
                mesh->TranslateLocal({ 0, 0, -5});
                mesh->UpdateTransformMatrix();
            }
            pinkBoxGeometry->TranslateLocal({ -5, 0, -5 });

            meshes.push_back(pinkBoxGeometry);
            meshes.insert(meshes.end(), gltfModel.begin(), gltfModel.end());
            meshes.insert(meshes.end(), cubemap.begin(), cubemap.end());
            meshes.push_back(grassQuad);

            for(auto mesh : meshes)
            {
                vkRenderer->AddMesh(mesh);
            }

            colorMaterial->SetColor(unicorn::video::Color::Blue());

            pWindow0->MousePosition.connect(&onCursorPositionChanged);
            pWindow0->Scroll.connect(&onMouseScrolled);
            pWindow0->Keyboard.connect(&onWindowKeyboard);
            pWindow0->MouseButton.connect(&onMouseButton);

            unicornRender->Run();
        }
    }

    delete pCameraFpsController;
    delete pCamera2DController;
    delete pPerspectiveProjection;
    delete pOrthoProjection;
    delete ortho;
    for(auto* mesh : meshes)
    {
        delete mesh;
    }
    unicornRender->Deinit();
    delete unicornRender;

    unicorn::Settings::Destroy();
}
