#include "project_selector_panel.h"
#include "theme.h"

#include <cstring>
#include <filesystem>

#include <imgui.h>

namespace dev_dash::ui
{
    ProjectSelectorPanel::ProjectSelectorPanel(SDL_Window* window,
                                               OnSelectedCallback onSelected)
        : _window(window)
        , _onSelected(std::move(onSelected))
    {
    }

    void ProjectSelectorPanel::Render()
    {
        // Drain pending path from dialog callback (called on main thread by SDL).
        if (_hasPendingPath)
        {
            const std::size_t copyLen =
                std::min(_pendingPath.size(), _pathBuffer.size() - 1);
            std::memcpy(_pathBuffer.data(), _pendingPath.data(), copyLen);
            _pathBuffer[copyLen] = '\0';
            _hasPendingPath = false;
            _errorMessage.clear();
        }

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Always);
        ImGui::Begin("##selector",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

        ImGui::Spacing();
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("DevDash").x) * 0.5f);
        ImGui::TextUnformatted("DevDash");

        ImGui::Spacing();
        ImGui::SetCursorPosX(
            (ImGui::GetContentRegionAvail().x
             - ImGui::CalcTextSize("Select a project to inspect").x) * 0.5f);
        ImGui::TextDisabled("Select a project to inspect");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float browseWidth = 80.0f;
        const float spacing     = ImGui::GetStyle().ItemSpacing.x;
        const float inputWidth  = ImGui::GetContentRegionAvail().x - browseWidth - spacing;

        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputText("##path", _pathBuffer.data(), _pathBuffer.size(),
                             ImGuiInputTextFlags_EnterReturnsTrue))
            TryConfirm();

        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(browseWidth, 0)))
        {
            const char* home = std::getenv("HOME");
            SDL_ShowOpenFolderDialog(
                &ProjectSelectorPanel::DialogCallback,
                this,
                _window,
                home ? home : nullptr,
                false);
        }

        ImGui::Spacing();

        const float btnWidth = 140.0f;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - btnWidth) * 0.5f);
        if (ImGui::Button("Open Project", ImVec2(btnWidth, 0)))
            TryConfirm();

        if (!_errorMessage.empty())
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().removed);
            ImGui::TextWrapped("%s", _errorMessage.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::End();
    }

    void ProjectSelectorPanel::TryConfirm()
    {
        const std::filesystem::path path(_pathBuffer.data());
        if (path.empty())
        {
            _errorMessage = "Please enter a project path.";
            return;
        }
        if (!std::filesystem::exists(path))
        {
            _errorMessage = "Path does not exist: " + path.string();
            return;
        }
        if (!std::filesystem::is_directory(path))
        {
            _errorMessage = "Path is not a directory: " + path.string();
            return;
        }
        _errorMessage.clear();
        _onSelected(path);
    }

    void SDLCALL ProjectSelectorPanel::DialogCallback(void* userdata,
                                                      const char* const* filelist,
                                                      int /*filter*/)
    {
        auto* self = static_cast<ProjectSelectorPanel*>(userdata);
        if (!filelist || !filelist[0])
            return;
        self->_pendingPath     = filelist[0];
        self->_hasPendingPath  = true;
    }
}
