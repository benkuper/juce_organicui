#include "juce_organicui.h"

#if ORGANICUI_USE_MCP
#include "mcp/McpCore.cpp"
#include "mcp/McpServer.cpp"
#include "mcp/OrganicMcpProvider.cpp"
#include "mcp/McpConnectionSettings.cpp"
#include "mcp/McpConnectionPanel.cpp"
#endif
#if ORGANICUI_USE_ASSISTANT
#include "assistant/AssistantCore.cpp"
#include "assistant/AssistantProvider.cpp"
#include "assistant/ClaudeProvider.cpp"
#include "assistant/AssistantController.cpp"
#include "assistant/AssistantPanel.cpp"
#endif

#include "comment/Comment.cpp"
#include "comment/CommentManager.cpp"
#include "comment/ui/CommentUI.cpp"
#include "comment/ui/CommentManagerViewUI.cpp"

#include "dashboard/DashboardFeedbackBroadcaster.cpp"
#include "dashboard/Dashboard.cpp"
#include "dashboard/DashboardItem.cpp"
#include "dashboard/DashboardCommentItem.cpp"
#include "dashboard/DashboardGroupItem.cpp"
#include "dashboard/DashboardLinkItem.cpp"
#include "dashboard/DashboardIFrameItem.cpp"
#include "dashboard/DashboardItemFactory.cpp" 
#include "dashboard/DashboardItemManager.cpp"
#include "dashboard/DashboardManager.cpp"


#if ORGANICUI_USE_SHAREDTEXTURE
#include "dashboard/SharedTextureDashboardItem.cpp"
#include "dashboard/ui/SharedTextureDashboardItemUI.cpp"
#endif
