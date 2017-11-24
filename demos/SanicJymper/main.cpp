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
#include <glm/gtc/matrix_transform.hpp>

#include <ctime>
#include <array>
#include <cstdlib>
#include <iostream>
#include <list>

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
const float zSens = 0.05f;

//Camera settings
bool isPerspective = true;
unicorn::video::Camera* perspective = nullptr;
unicorn::video::Camera* ortho = nullptr;
unicorn::video::CameraFpsController* pCameraFpsController = nullptr;
unicorn::video::Camera2DController* pCamera2DController = nullptr;
unicorn::video::PerspectiveCamera* pPerspectiveProjection = nullptr;
unicorn::video::OrthographicCamera* pOrthoProjection = nullptr;

static unicorn::video::Mesh* earth;
static unicorn::video::Mesh* sun;

void onLogicFrame(unicorn::UnicornRender* /*render*/)
{
    float currentFrame = static_cast<float>(timer->ElapsedMilliseconds().count()) / 1000;
    float newDeltatime = currentFrame - lastFrame;

    if(newDeltatime <= 0.0)
    {
        return;
    }

    deltaTime = newDeltatime;

    // Applying transforms
    earth->RotateAroundPoint(static_cast<float>(glm::radians(90.)) * deltaTime , { 0, 1, 0 }, sun->GetTranslate());
    sun->RotateAroundPoint(static_cast<float>(glm::radians(90.)) * deltaTime, { 0, 1, 0 }, earth->GetTranslate());

    // Updating transformations for meshes
    for(auto& mesh : meshes)
    {
        mesh->Calculate();
    }

    // Updating transformations for cameras
    pCameraFpsController->Calculate();
    pCamera2DController->Calculate();

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
            unicorn::video::Material cubematerial;
            cubematerial.color = {static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255};
            unicorn::video::Mesh* mesh = &unicorn::video::Primitives::Sphere(*vkRenderer->SpawnMesh(cubematerial), 1, 32, 32);
            mesh->Translate({ std::rand() % 40 - 20, std::rand() % 40 - 20, std::rand() % 40 - 20 });
            meshes.push_back(mesh);
            break;
        }
        case MouseButton::MouseMiddle:
        {
            if(meshes.size())
            {
                // Get random cube
                auto meshIt = meshes.begin();

                std::advance(meshIt, std::rand() % meshes.size());

                auto mesh = *meshIt;

                unicorn::video::Material cubeMaterial;
                cubeMaterial.color = {static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255, static_cast<float>(std::rand() % 255) / 255};
                cubeMaterial.SetIsWired(true);
                cubeMaterial.SetIsVisible(false);
                mesh->SetMaterial(cubeMaterial);
            }
            break;
        }
        case MouseButton::MouseRight:
        {
            if(meshes.size())
            {
                // Get random cube
                auto meshIt = meshes.begin();

                std::advance(meshIt, std::rand() % meshes.size());

                auto mesh = *meshIt;

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
        pPerspectiveProjection->Zoom(static_cast<float>(pos.second / 50)); // 50 is zoom coefficient
    }
    else
    {
        pOrthoProjection->Scale(static_cast<float>(pos.second * 10)); // 10 is scale speed
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
    unicorn::system::input::Key const& key = keyboardEvent.key;
    uint32_t const& scancode = keyboardEvent.scancode;
    unicorn::system::input::Modifier::Mask const& modifiers = keyboardEvent.modifiers;

    std::pair<int32_t, int32_t> position = pWindow->GetPosition();

    float time = deltaTime * 0.1f;
    float speed = 1000.f;

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
                pCameraFpsController->TranslateLocalZ(time * speed);
            }
            else
            {
                pCamera2DController->TranslateLocalY(time * speed);
            }
            break;
        }
        case Key::S:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocalZ(-time * speed);
            }
            else
            {
                pCamera2DController->TranslateLocalY(-time * speed);
            }
            break;
        }
        case Key::A:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocalX(time * speed);
            }
            else
            {
                pCamera2DController->TranslateLocalX(time * speed);
            }
            break;
        }
        case Key::D:
        {
            if (isPerspective)
            {
                pCameraFpsController->TranslateLocalX(-time * speed);
            }
            else
            {
                pCamera2DController->TranslateLocalX(-time * speed);
            }
            break;
        }
        case Key::N:
        {
            if (isPerspective)
            {
                pCameraFpsController->RotateY(glm::radians(1.f));
            }
            break;
        }
        case Key::Q:
        {
            if (isPerspective)
            {
                pCameraFpsController->RotateZ(static_cast<float>(-glm::radians(1.)));
            }
            break;
        }
        case Key::E:
        {
            if (isPerspective)
            {
                pCameraFpsController->RotateZ(static_cast<float>(glm::radians(1.)));
            }
            break;
        }
         case Key::Space:
         {
             if (isPerspective)
             {
                 pCameraFpsController->TranslateLocalY(time * speed);
             }
             break;
         }
         case Key::LeftControl:
         {
             if (isPerspective)
             {
                 pCameraFpsController->TranslateLocalY(-time * speed);
             }
             break;
         }
        case Key::Up:
        {
            pCameraFpsController->RotateX(static_cast<float>(glm::radians(-1.)));
            break;
        }
        case Key::Down:
        {
            pCameraFpsController->RotateX(static_cast<float>(glm::radians(1.)));
            break;
        }
        case Key::Left:
        {
            pCameraFpsController->RotateY(static_cast<float>(glm::radians(1.)));
            break;
        }
        case Key::Right:
        {
            pCameraFpsController->RotateY(static_cast<float>(glm::radians(-1.)));
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

bool MakeCubeMap()
{
    //Loading textures
    unicorn::video::Texture texture, textureMandrill;
    unicorn::video::Texture frontSkyBox, backSkyBox, leftSkyBox, rightSkyBox, topSkyBox, bottomSkyBox;

    frontSkyBox.Load("data/textures/cubemap/posz.jpg");
    backSkyBox.Load("data/textures/cubemap/negz.jpg");
    leftSkyBox.Load("data/textures/cubemap/negx.jpg");
    rightSkyBox.Load("data/textures/cubemap/posx.jpg");
    topSkyBox.Load("data/textures/cubemap/posy.jpg");
    bottomSkyBox.Load("data/textures/cubemap/negy.jpg");

    texture.Load("data/textures/texture.jpg");
    textureMandrill.Load("data/textures/mandrill.png");

    //Checks data
    if (!texture.IsLoaded()
        || !textureMandrill.IsLoaded()
        || !frontSkyBox.IsLoaded()
        || !backSkyBox.IsLoaded()
        || !leftSkyBox.IsLoaded()
        || !rightSkyBox.IsLoaded()
        || !topSkyBox.IsLoaded()
        || !bottomSkyBox.IsLoaded())
    {
        return false;
    }

    unicorn::video::Material textureMaterial,
        mandrillMaterial, frontTexture, backTexture,
        leftTexture, rightTexture, upTexture, bottomTexture;

    textureMaterial.SetAlbedo(&texture);
    mandrillMaterial.SetAlbedo(&textureMandrill);
    frontTexture.SetAlbedo(&frontSkyBox);
    backTexture.SetAlbedo(&backSkyBox);
    leftTexture.SetAlbedo(&leftSkyBox);
    rightTexture.SetAlbedo(&rightSkyBox);
    upTexture.SetAlbedo(&topSkyBox);
    bottomTexture.SetAlbedo(&bottomSkyBox);

    using unicorn::video::Primitives;

    unicorn::video::Mesh* frontBox = &Primitives::Quad(*vkRenderer->SpawnMesh(frontTexture));
    unicorn::video::Mesh* backBox = &Primitives::Quad(*vkRenderer->SpawnMesh(backTexture));
    unicorn::video::Mesh* leftBox = &Primitives::Quad(*vkRenderer->SpawnMesh(leftTexture));
    unicorn::video::Mesh* rightBox = &Primitives::Quad(*vkRenderer->SpawnMesh(rightTexture));
    unicorn::video::Mesh* upBox = &Primitives::Quad(*vkRenderer->SpawnMesh(upTexture));
    unicorn::video::Mesh* bottomBox = &Primitives::Quad(*vkRenderer->SpawnMesh(bottomTexture));

    meshes.push_back(frontBox);
    meshes.push_back(backBox);
    meshes.push_back(leftBox);
    meshes.push_back(rightBox);
    meshes.push_back(upBox);
    meshes.push_back(bottomBox);

    //Skybox
    const float skyBoxScaleFactor = 500;
    const float skyBoxDistance = skyBoxScaleFactor / 2 - 1;

    frontBox->Translate({ 0, 0, skyBoxDistance });
    backBox->Translate({ 0, 0, -skyBoxDistance });
    backBox->Rotate(static_cast<float>(glm::radians(-180.0)), { 0.0, 1.0, 0.0 });

    upBox->Translate({ 0, skyBoxDistance, 0 });
    upBox->Rotate(static_cast<float>(glm::radians(-90.0)), { 1.0, 0.0, 0.0 });
    bottomBox->Translate({ 0, -skyBoxDistance, 0 });
    bottomBox->Rotate(static_cast<float>(glm::radians(90.0)), { 1.0, 0.0, 0.0 });

    leftBox->Translate({ -skyBoxDistance, 0, 0 });
    leftBox->Rotate(static_cast<float>(glm::radians(-90.0)), { 0.0, 1.0, 0.0 });
    rightBox->Translate({ skyBoxDistance, 0, 0 });
    rightBox->Rotate(static_cast<float>(glm::radians(90.0)), { 0.0, 1.0, 0.0 });

    frontBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    backBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    upBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    bottomBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    leftBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });
    rightBox->Scale({ skyBoxScaleFactor, skyBoxScaleFactor, 0 });

    return true;
}

int main(int argc, char* argv[])
{
    unicorn::Settings& settings = unicorn::Settings::Instance();

    settings.Init(argc, argv, "Sanic_Jymper.log");
    settings.SetApplicationName("SANIC JYMPER");
    unicorn::UnicornRender* unicornRender = new unicorn::UnicornRender();
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

        auto h = pGraphics->GetMonitors().back()->GetActiveVideoMode().height;
        auto w = pGraphics->GetMonitors().back()->GetActiveVideoMode().width;
        settings.SetApplicationHeight(h);
        settings.SetApplicationWidth(w);

        unicorn::system::Window* pWindow0 = pGraphics->SpawnWindow(settings.GetApplicationWidth(),
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
        pPerspectiveProjection = new unicorn::video::PerspectiveCamera(pWindow0, perspective->projection);
        pOrthoProjection = new unicorn::video::OrthographicCamera(pWindow0, ortho->projection);

        pCameraFpsController = new unicorn::video::CameraFpsController(perspective->view);
        pCamera2DController = new unicorn::video::Camera2DController(ortho->view);

        {
            //Loading textures
            unicorn::video::Texture texture, textureMandrill;

            texture.Load("data/textures/texture.jpg");
            textureMandrill.Load("data/textures/mandrill.png");

            if (!texture.IsLoaded()
                || !textureMandrill.IsLoaded())
            {
                return -1;
            }

            if (!MakeCubeMap())
            {
                return -1;
            }

            using unicorn::video::Primitives;

            unicorn::video::Material mat;

            mat.color = unicorn::video::Color::LightPink();
            auto x_plus = &Primitives::Sphere(*vkRenderer->SpawnMesh(mat), 40, 16, 16);

            // auto gltfMesh = &Primitives::LoadMeshFromFile(*vkRenderer->SpawnMesh(mat),
            //     "data/models/glTF/DamagedHelmet/DamagedHelmet.gltf");

            auto objMesh = &Primitives::LoadMeshFromFile(*vkRenderer->SpawnMesh(mat),
                "data/models/obj/dragon.obj");

            mat.color = unicorn::video::Color::Green();
            auto x_minus = &Primitives::Sphere(*vkRenderer->SpawnMesh(mat), 40, 16, 16);

            mat.color = unicorn::video::Color::Blue();
            auto z_plus = &Primitives::Sphere(*vkRenderer->SpawnMesh(mat), 40, 16, 16);

            x_minus->Translate({ -250, 0, 0 });
            x_plus->Translate({ 250, 0, 0 });
            z_plus->Translate({ 0, 0, 250 });

            mat.color = unicorn::video::Color::Red();
            sun = &Primitives::Sphere(*vkRenderer->SpawnMesh(mat), 1, 16, 16);
            sun->Scale({ 1, 1, 1 });
            mat.color = unicorn::video::Color::Green();
            mat.SetAlbedo(&texture);
            earth = &Primitives::Sphere(*vkRenderer->SpawnMesh(mat), 1, 16, 16);
            earth->Scale({ 2, 2, 2 });
            earth->Translate({ -15, 0, 50 });
            sun->Translate({ 15, 0, 50 });

            objMesh->Translate({5, 0, 0});

            unicorn::video::Mesh* box = &Primitives::Box(*vkRenderer->SpawnMesh(mat));

            meshes.push_back(box);
            meshes.push_back(earth);
            meshes.push_back(sun);
            meshes.push_back(x_plus);
            meshes.push_back(x_minus);
            meshes.push_back(z_plus);
            meshes.push_back(objMesh);

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

    unicornRender->Deinit();
    delete unicornRender;

    unicorn::Settings::Destroy();
}
