#include "document_loader.h"

#include <fstream>
#include <iostream>
#include <string_view>

namespace dev_dash::services
{
    LoadResult DocumentLoader::Load(const std::filesystem::path& filePath)
    {
        std::ifstream file(filePath);
        if (!file)
        {
            std::cerr << "[warn] DocumentLoader: could not open: " << filePath << '\n';
            return {};
        }

        const std::filesystem::path dir = filePath.parent_path();
        LoadResult result;
        std::string line;
        bool isCodeBlock = false;

        while (std::getline(file, line))
        {
            if (line.starts_with("```"))
                isCodeBlock = !isCodeBlock;

            const auto firstNonSpace = line.find_first_not_of(" \t");
            if (!isCodeBlock
                && firstNonSpace != std::string::npos
                && line[firstNonSpace] == '@'
                && firstNonSpace + 1 < line.size()
                && line[firstNonSpace + 1] != ' ')
            {
                const std::string importPath = line.substr(firstNonSpace + 1);
                const auto resolved = std::filesystem::weakly_canonical(dir / importPath);
                const std::string label = resolved.filename().string();
                result.content += "[" + label + "](claudeimport://" + resolved.string() + ")\n";
                result.imports.push_back(resolved);
            }
            else
            {
                result.content += line + '\n';
            }
        }

        return result;
    }
}
