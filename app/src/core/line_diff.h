#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace dev_dash::core
{
    enum class DiffLineKind { kContext, kAdded, kRemoved };

    struct DiffLine
    {
        DiffLineKind kind;
        std::string  text;
    };

    inline std::vector<std::string> SplitLines(const std::string& text)
    {
        std::vector<std::string> lines;
        std::size_t start = 0;
        for (std::size_t i = 0; i <= text.size(); ++i)
        {
            if (i == text.size() || text[i] == '\n')
            {
                // Strip trailing '\r' for CRLF files.
                const std::size_t end =
                    (i > start && text[i - 1] == '\r') ? i - 1 : i;
                lines.push_back(text.substr(start, end - start));
                start = i + 1;
            }
        }
        if (!lines.empty() && lines.back().empty())
            lines.pop_back();
        return lines;
    }

    // Compute a line-by-line LCS diff of textA vs textB.
    // kRemoved = in A only, kAdded = in B only, kContext = in both.
    // Returns an empty vector if either file exceeds kMaxLines (too large).
    inline std::vector<DiffLine> DiffLines(const std::string& textA,
                                            const std::string& textB)
    {
        constexpr int kMaxLines = 2000;

        const auto linesA = SplitLines(textA);
        const auto linesB = SplitLines(textB);
        const int  m      = static_cast<int>(linesA.size());
        const int  n      = static_cast<int>(linesB.size());

        if (m > kMaxLines || n > kMaxLines)
            return {};

        // DP table: dp[i][j] = LCS length of linesA[0..i-1], linesB[0..j-1].
        std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
        for (int i = 1; i <= m; ++i)
            for (int j = 1; j <= n; ++j)
                dp[i][j] = (linesA[i - 1] == linesB[j - 1])
                                ? dp[i - 1][j - 1] + 1
                                : std::max(dp[i - 1][j], dp[i][j - 1]);

        // Backtrack to produce the diff in reverse, then flip.
        std::vector<DiffLine> result;
        result.reserve(m + n);
        int i = m, j = n;
        while (i > 0 || j > 0)
        {
            if (i > 0 && j > 0 && linesA[i - 1] == linesB[j - 1])
            {
                result.push_back({DiffLineKind::kContext, linesA[i - 1]});
                --i; --j;
            }
            else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j]))
            {
                result.push_back({DiffLineKind::kAdded, linesB[j - 1]});
                --j;
            }
            else
            {
                result.push_back({DiffLineKind::kRemoved, linesA[i - 1]});
                --i;
            }
        }

        std::reverse(result.begin(), result.end());
        return result;
    }
}
