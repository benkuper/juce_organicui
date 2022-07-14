#include "juce_organicui.h"

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
#include "dashboard/DashboardItemFactory.cpp" 
#include "dashboard/DashboardItemManager.cpp"
#include "dashboard/DashboardManager.cpp"


#if ORGANICUI_USE_SHAREDTEXTURE
#include "dashboard/SharedTextureDashboardItem.cpp"
#include "dashboard/ui/SharedTextureDashboardItemUI.cpp"
#endif