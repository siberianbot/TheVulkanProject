#ifndef ENGINE_VARS_HPP
#define ENGINE_VARS_HPP

#include <string_view>

static constexpr const char *WINDOW_TITLE_VAR = "Window.Title";
static constexpr const char *WINDOW_WIDTH_VAR = "Window.Width";
static constexpr const char *WINDOW_HEIGHT_VAR = "Window.Height";

static constexpr const std::string_view RENDERING_VSYNC = "Rendering.VSync";

static constexpr const char *RENDERING_SCENE_STAGE_SHADOW_MAP_SIZE = "Rendering.SceneStage.ShadowMapSize";
static constexpr const char *RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT = "Rendering.SceneStage.ShadowMapCount";
static constexpr const char *RENDERING_SCENE_STAGE_LIGHT_COUNT = "Rendering.SceneStage.LightCount";

static constexpr const char *RESOURCES_DEFAULT_TEXTURE = "Resources.DefaultTexture";

#endif // ENGINE_VARS_HPP
