#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
"Active", 
"Active / Search String", 
"&Add", 
"Added", 
"Add To Favorites", 
"Address already in use", 
"Address not available", 
"Discard", 
"Automatic Directory Listing Search", 
"All download slots taken", 
"All %d users offline", 
"All 3 users offline", 
"All 4 users offline", 
"Any", 
"At least", 
"At most", 
"Audio", 
"Auto connect / Name", 
"Auto grant slot / Nick", 
"Average/s: ", 
"AWAY", 
"Away mode off", 
"Away mode on: ", 
"B", 
"Both users offline", 
"Browse...", 
"&Browse...", 
"Choose folder", 
"Close connection", 
"Closing connection...", 
"Close", 
"Compressed", 
"Error during compression", 
"&Connect", 
"Connected", 
"Connecting...", 
"Connecting (forced)...", 
"Connecting to ", 
"Connection", 
"Connection closed", 
"Connection refused by target machine", 
"Connection reset by server", 
"Connection timeout", 
"Copy nick to clipboard", 
"Could not open target file: ", 
"Count", 
"CRC Checked", 
"Error during decompression", 
"Description", 
"Destination", 
"Directory", 
"Directory already shared", 
"Disconnected", 
"Disk full(?)", 
"Document", 
"Done", 
"The temporary download directory cannot be shared", 
"Download", 
"Download failed: ", 
"Download finished, idle...", 
"Download starting...", 
"Download to...", 
"Download queue", 
"Download whole directory", 
"Download whole directory to...", 
"Downloaded", 
"Downloaded %s (%.01f%%) in %s", 
" downloaded from ", 
"Downloading...", 
"Downloading public hub list...", 
"Downloads", 
"Duplicate source", 
"Edit", 
"E-Mail", 
"Please enter a nickname in the settings dialog!", 
"Please enter a password", 
"Please enter a reason", 
"Enter search string", 
"Please enter a destination server", 
"Error opening file", 
"Errors", 
"Exact size", 
"Executable", 
"Favorite Hubs", 
"Favorite hub added", 
"Favorite Users", 
"Favorite user added", 
"File", 
"Files", 
"File list refreshed", 
"File not available", 
"File type", 
"A file with a different size already exists in the queue", 
"A file with diffent tth root already exists in the queue", 
"Filename", 
"F&ilter", 
"Find", 
"Finished Downloads", 
"Finished Uploads", 
"Force attempt", 
"GB", 
"Get file list", 
"Go to directory", 
"Grant extra slot", 
"Finished hashing ", 
"High", 
"Highest", 
"Hit Ratio: ", 
"Hits: ", 
"Host unreachable", 
"Hub", 
"Hubs", 
"Address", 
"Hub list downloaded...", 
"Name", 
"Hub password", 
"Users", 
"Ignored message: ", 
"Invalid number of slots", 
"Invalid target file (missing directory, check default download directory setting)", 
"Ip: ", 
"Items", 
"Joins: ", 
"Join/part showing off", 
"Join/part showing on", 
"kB", 
"kB/s", 
"Kick user(s)", 
"A file of equal or larger size already exists at the target location", 
"Last change: ", 
"Hub (last seen on if offline)", 
"Time last seen", 
"Loading DC++, please wait...", 
"Low", 
"Lowest", 
"Manual connect address", 
"Match queue", 
"Matched %d file(s)", 
"MB", 
"&File", 
"ADL Search", 
"&Download Queue\tCtrl+D", 
"&Exit", 
"&Favorite Hubs\tCtrl+F", 
"Favorite &Users\tCtrl+U", 
"Follow last redirec&t\tCtrl+T", 
"Import queue from NMDC...", 
"Network Statistics", 
"&Notepad\tCtrl+N", 
"Open file list...", 
"&Public Hubs\tCtrl+P", 
"&Reconnect\tCtrl+R", 
"Refresh file list", 
"Show", 
"&Search\tCtrl+S", 
"Search spy", 
"Settings...", 
"&Help", 
"About DC++...", 
"Change Log", 
"DC++ discussion forum", 
"Donate ���/$$$ (paypal)", 
"Downloads and translations", 
"Frequently asked questions", 
"Help forum", 
"DC++ Homepage", 
"Open downloads directory", 
"Readme / Newbie help", 
"Request a feature", 
"Report a bug", 
"&View", 
"&Status bar Ctrl+2", 
"&Toolbar Ctrl+1", 
"&Window", 
"Arrange icons", 
"Cascade", 
"Horizontal Tile", 
"Vertical Tile", 
"Close disconnected", 
"Minimize &All", 
"Move/Rename", 
"Move &Down", 
"Move &Up", 
"Network Statistics", 
"Network unreachable (are you connected to the internet?)", 
"Next", 
"&New...", 
"Nick", 
"Your nick was already taken, please change to something else!", 
" (Nick unknown)", 
"Non-blocking operation still in progress", 
"Not connected", 
"Not a socket", 
"No", 
"No directory specified", 
"You're trying to download from yourself!", 
"No errors", 
"No matches", 
"No slots available", 
"No users", 
"No users to download from", 
"Normal", 
"Notepad", 
"Offline", 
"Online", 
"Only users with free slots", 
"Only where I'm op", 
"Open", 
"Open download page?", 
"Open folder", 
"Operation would block execution", 
"Out of buffer space", 
"Passive user", 
"Password", 
"Parts: ", 
"Path", 
"Paused", 
"Permission denied", 
"Picture", 
"Port: ", 
"Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", 
"Preparing file list...", 
"Press the follow redirect button to connect to ", 
"Priority", 
"Private message from ", 
"&Properties", 
"Public Hubs", 
"Ratio", 
"Readd source", 
"Really exit?", 
"Redirect", 
"Redirect request received to a hub that's already connected", 
"Redirect user(s)", 
"&Refresh", 
"Refresh user list", 
"&Remove", 
"Remove all", 
"Remove all subdirectories before adding this one", 
"Remove user from queue", 
"Remove source", 
"Rollback inconsistency, existing file does not match the one being downloaded", 
"Running...", 
"Search", 
"Search for", 
"Search for alternates", 
"Search for file", 
"Search options", 
"Search spam detected from ", 
"Search Spy", 
"Search String", 
"Specifying the same search string for more than 5 files for a passive connection or 10 files for an active connection is inefficient. Would you like to continue with the operation?", 
"Searching for ", 
"Send private message", 
"Separator", 
"Server", 
"Set priority", 
"&Add folder", 
"Break on first ADLSearch match", 
"Advanced", 
"Advanced settings", 
"Use antifragmentation method for downloads", 
"Appearance", 
"Auto-away on minimize (and back on restore)", 
"Automatically follow redirects", 
"Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", 
"Automatically search for alternative download locations", 
"Use exact filename when matching autosearches", 
"Automatically match queue for auto search hits", 
"Use default search strings in auto search when no string is specified", 
"Automatically refresh share list every hour", 
"&Change", 
"Clear search box after each search", 
"Client version", 
"Colors", 
"Enable safe and compressed transfers", 
"Command", 
"Confirm application exit", 
"Connection Settings (see the readme / newbie help if unsure)", 
"Connection Type", 
"Default away message", 
"Directories", 
"Default download directory", 
"Limits", 
"Downloads", 
"Maximum simultaneous downloads (0 = infinite)", 
"No new downloads if speed exceeds (kB/s, 0 = disable)", 
"Donate $$$:s! (well, ��� preferably =) (see help menu)", 
"Filter kick and NMDC debug messages", 
"Set Finished Manager(s) tab bold when an entry is added", 
"Full row select in lists", 
"General", 
"Enable TTH file hash features", 
"Accept custom user commands from hub", 
"Ignore messages from users that are not online (effective against bots)", 
"IP", 
"Don't delete file lists when exiting", 
"Language file", 
"Log directory", 
"Log downloads", 
"Log file list transfers", 
"Log main chat", 
"Log private chat", 
"Log uploads", 
"Logging", 
"Logs and Sound", 
"Max tab rows", 
"Minimize to tray", 
"Name", 
"Don't send the away message to bots", 
"Open the favorite hubs window at startup", 
"Open the finished downloads window at startup", 
"Open the public hubs window at startup", 
"Open the download queue window at startup", 
"Options", 
"Passive", 
"Personal Information", 
"Make an annoying sound every time a private message is received", 
"Make an annoying sound when a private message window is opened", 
"Popup messages from users that are not online (if not ignoring, messages go to main chat if enabled)", 
"Popup private messages", 
"Port (empty=random)", 
"Public Hubs list", 
"Public Hubs list URL", 
"HTTP Proxy (for hublist only)", 
"Remove dupes completely from your share (otherwise, only their size is subtracted, but the files can be seen by others)", 
"Note; most of these options require that you restart DC++", 
"Rollback", 
"Select &text style", 
"Select &window color", 
"Enable automatic SFV checking", 
"Share hidden files", 
"Total size:", 
"Shared directories", 
"Show joins / parts in chat by default", 
"Show progress bars for transfers (uses some CPU)", 
"Skip zero-byte files", 
"Use small send buffer (enable if uploads slow downloads a lot)", 
"SOCKS5", 
"Socks IP", 
"Port", 
"Use SOCKS5 server to resolve hostnames", 
"Username", 
"Sounds", 
"Note; because of changing download speeds, this is not 100% accurate...", 
"View status messages in main chat", 
"Tab completion of nicks in chat", 
"Show timestamps in chat by default", 
"Unfinished downloads directory (empty = download directly to target)", 
"Sharing", 
"Automatically open an extra slot if speed is below (0 = disable)", 
"Upload slots", 
"Install URL handler on startup (to handle dchub:// links)", 
"Use OEM monospaced font for viewing text files", 
"Use system icons when browsing files (slows browsing down a bit)", 
"Advanced\\User Commands", 
"Write buffer size", 
"CRC32 inconsistency (SFV-Check)", 
"Shared", 
"Shared Files", 
"Size", 
"Max Size", 
"Min Size", 
"Slot granted", 
"Slots", 
"Slots set", 
"Socket has been shut down", 
"Socks server authentication failed (bad username / password?)", 
"The socks server doesn't support user / password authentication", 
"The socks server failed establish a connection", 
"The socks server requires authentication", 
"Failed to set up the socks server for UDP relay (check socks address and port)", 
"Source Type", 
"Specify a server to connect to", 
"Specify a search string", 
"Speed", 
"Status", 
"Stored password sent...", 
"Tag", 
"Target filename too long", 
"TB", 
"Time", 
"Time left", 
"Timestamps disabled", 
"Timestamps enabled", 
"Total: ", 
"TTH Root", 
"Type", 
"Unable to create thread", 
"Unknown", 
"Unknown address", 
"Unknown error: 0x%x", 
"Unsupported filelist format", 
"Upload finished, idle...", 
"Upload starting...", 
"Uploaded %s (%.01f%%) in %s", 
" uploaded to ", 
"Uploads", 
"User", 
"User Description", 
"User offline", 
"User went offline", 
"Users", 
"Video", 
"View as text", 
"Waiting...", 
"Waiting (User online)", 
"Waiting (%d of %d users online)", 
"Waiting to retry...", 
"What's &this?", 
"Yes", 
"You are being redirected to ", 
"Copy to clipboard", 
"Auto update incoming", 
"Expand Queue", 
"Search Sites", 
"Transfer View\tCtrl+3", 
"Strip ISP (public hubs only, see favorite hubs settings)", 
"Strip ISP in private messages", 
"ISP", 
"Make tabs bold on status messages in Hubs", 
"Make tabs bold on status messages in private messages", 
"Make tabs bold on updates in download queue", 
"Set high priority on sample and sub files when enqueued", 
"Create new main chat log every midnight", 
"Only show popups if away", 
"Show popup on PM", 
"Show popup on PM only if it's from a new user", 
"Show popup on hub status messages such as disconnected", 
"Confirm hub exit", 
"Confirm removal of files from download queue", 
"Copy", 
"Can't add an empty string", 
"Badly formatted regular expression", 
"All Uploads", 
"All Downloads", 
"Popups turned on", 
"Popups turned off", 
"View Log", 
"FulDC\\Advanced", 
"FulDC\\Highlight", 
"FulDC\\More", 
"Open private messages in background", 
"Open file lists in background", 
"Url", 
"Key", 
"&OK", 
"&Cancel", 
"Web shortcuts properties", 
"How to use", 
"Web shortcuts are used to open web pages with specific contents. The key can be more than one character, but must not contain any space. The URL has to have exactly One \"%s\" which will be replaced with the string you'll enter. If no \"%s\" exists, nothing will be pasted to the url, and the page will just open up. An example is:\nhttp://domain.com/search=%s\n\nNote: The key is case sensitive!", 
"Name required", 
"Name already in use", 
"Key already in use", 
"Active background color", 
"Active text color", 
"Active border color", 
"Inactive background color", 
"Show icons in tabs", 
"Disconnected/Offline PM color", 
"Inactive text color", 
"Inactive border color", 
"Inactive notification color", 
"Finished searching the document", 
"User hasn't been seen", 
"Removed", 
"Directory not shared", 
"fulDC Homepage", 
"Close all PM windows", 
"Close all file lists", 
"Play custom sound on PM's", 
"Muted", 
"Unmuted", 
};
string ResourceManager::names[] = {
"Active", 
"ActiveSearchString", 
"Add", 
"Added", 
"AddToFavorites", 
"AddressAlreadyInUse", 
"AddressNotAvailable", 
"AdlDiscard", 
"AdlSearch", 
"AllDownloadSlotsTaken", 
"AllUsersOffline", 
"All3UsersOffline", 
"All4UsersOffline", 
"Any", 
"AtLeast", 
"AtMost", 
"Audio", 
"AutoConnect", 
"AutoGrant", 
"Average", 
"Away", 
"AwayModeOff", 
"AwayModeOn", 
"B", 
"BothUsersOffline", 
"Browse", 
"BrowseAccel", 
"ChooseFolder", 
"CloseConnection", 
"ClosingConnection", 
"Close", 
"Compressed", 
"CompressionError", 
"Connect", 
"Connected", 
"Connecting", 
"ConnectingForced", 
"ConnectingTo", 
"Connection", 
"ConnectionClosed", 
"ConnectionRefused", 
"ConnectionReset", 
"ConnectionTimeout", 
"CopyNick", 
"CouldNotOpenTargetFile", 
"Count", 
"CrcChecked", 
"DecompressionError", 
"Description", 
"Destination", 
"Directory", 
"DirectoryAlreadyShared", 
"Disconnected", 
"DiscFull", 
"Document", 
"Done", 
"DontShareTempDirectory", 
"Download", 
"DownloadFailed", 
"DownloadFinishedIdle", 
"DownloadStarting", 
"DownloadTo", 
"DownloadQueue", 
"DownloadWholeDir", 
"DownloadWholeDirTo", 
"Downloaded", 
"DownloadedBytes", 
"DownloadedFrom", 
"Downloading", 
"DownloadingHubList", 
"Downloads", 
"DuplicateSource", 
"Edit", 
"Email", 
"EnterNick", 
"EnterPassword", 
"EnterReason", 
"EnterSearchString", 
"EnterServer", 
"ErrorOpeningFile", 
"Errors", 
"ExactSize", 
"Executable", 
"FavoriteHubs", 
"FavoriteHubAdded", 
"FavoriteUsers", 
"FavoriteUserAdded", 
"File", 
"Files", 
"FileListRefreshed", 
"FileNotAvailable", 
"FileType", 
"FileWithDifferentSize", 
"FileWithDifferentTth", 
"Filename", 
"Filter", 
"Find", 
"FinishedDownloads", 
"FinishedUploads", 
"ForceAttempt", 
"Gb", 
"GetFileList", 
"GoToDirectory", 
"GrantExtraSlot", 
"HashingFinished", 
"High", 
"Highest", 
"HitRatio", 
"Hits", 
"HostUnreachable", 
"Hub", 
"Hubs", 
"HubAddress", 
"HubListDownloaded", 
"HubName", 
"HubPassword", 
"HubUsers", 
"IgnoredMessage", 
"InvalidNumberOfSlots", 
"InvalidTargetFile", 
"Ip", 
"Items", 
"Joins", 
"JoinShowingOff", 
"JoinShowingOn", 
"Kb", 
"Kbps", 
"KickUser", 
"LargerTargetFileExists", 
"LastChange", 
"LastHub", 
"LastSeen", 
"Loading", 
"Low", 
"Lowest", 
"ManualAddress", 
"MatchQueue", 
"MatchedFiles", 
"Mb", 
"MenuFile", 
"MenuAdlSearch", 
"MenuDownloadQueue", 
"MenuExit", 
"MenuFavoriteHubs", 
"MenuFavoriteUsers", 
"MenuFollowRedirect", 
"MenuImportQueue", 
"MenuNetworkStatistics", 
"MenuNotepad", 
"MenuOpenFileList", 
"MenuPublicHubs", 
"MenuReconnect", 
"MenuRefreshFileList", 
"MenuShow", 
"MenuSearch", 
"MenuSearchSpy", 
"MenuSettings", 
"MenuHelp", 
"MenuAbout", 
"MenuChangelog", 
"MenuDiscuss", 
"MenuDonate", 
"MenuDownloads", 
"MenuFaq", 
"MenuHelpForum", 
"MenuHomepage", 
"MenuOpenDownloadsDir", 
"MenuReadme", 
"MenuRequestFeature", 
"MenuReportBug", 
"MenuView", 
"MenuStatusBar", 
"MenuToolbar", 
"MenuWindow", 
"MenuArrange", 
"MenuCascade", 
"MenuHorizontalTile", 
"MenuVerticalTile", 
"MenuCloseDisconnected", 
"MenuMinimizeAll", 
"Move", 
"MoveDown", 
"MoveUp", 
"NetworkStatistics", 
"NetworkUnreachable", 
"Next", 
"New", 
"Nick", 
"NickTaken", 
"NickUnknown", 
"NonBlockingOperation", 
"NotConnected", 
"NotSocket", 
"No", 
"NoDirectorySpecified", 
"NoDownloadsFromSelf", 
"NoErrors", 
"NoMatches", 
"NoSlotsAvailable", 
"NoUsers", 
"NoUsersToDownloadFrom", 
"Normal", 
"Notepad", 
"Offline", 
"Online", 
"OnlyFreeSlots", 
"OnlyWhereOp", 
"Open", 
"OpenDownloadPage", 
"OpenFolder", 
"OperationWouldBlockExecution", 
"OutOfBufferSpace", 
"PassiveUser", 
"Password", 
"Parts", 
"Path", 
"Paused", 
"PermissionDenied", 
"Picture", 
"Port", 
"PortIsBusy", 
"PreparingFileList", 
"PressFollow", 
"Priority", 
"PrivateMessageFrom", 
"Properties", 
"PublicHubs", 
"Ratio", 
"ReaddSource", 
"ReallyExit", 
"Redirect", 
"RedirectAlreadyConnected", 
"RedirectUser", 
"Refresh", 
"RefreshUserList", 
"Remove", 
"RemoveAll", 
"RemoveAllSubdirectories", 
"RemoveFromAll", 
"RemoveSource", 
"RollbackInconsistency", 
"Running", 
"Search", 
"SearchFor", 
"SearchForAlternates", 
"SearchForFile", 
"SearchOptions", 
"SearchSpamFrom", 
"SearchSpy", 
"SearchString", 
"SearchStringInefficient", 
"SearchingFor", 
"SendPrivateMessage", 
"Separator", 
"Server", 
"SetPriority", 
"SettingsAddFolder", 
"SettingsAdlsBreakOnFirst", 
"SettingsAdvanced", 
"SettingsAdvancedSettings", 
"SettingsAntiFrag", 
"SettingsAppearance", 
"SettingsAutoAway", 
"SettingsAutoFollow", 
"SettingsAutoKick", 
"SettingsAutoSearch", 
"SettingsAutoSearchExact", 
"SettingsAutoSearchAutoMatch", 
"SettingsAutoSearchAutoString", 
"SettingsAutoUpdateList", 
"SettingsChange", 
"SettingsClearSearch", 
"SettingsClientVer", 
"SettingsColors", 
"SettingsCompressTransfers", 
"SettingsCommand", 
"SettingsConfirmExit", 
"SettingsConnectionSettings", 
"SettingsConnectionType", 
"SettingsDefaultAwayMsg", 
"SettingsDirectories", 
"SettingsDownloadDirectory", 
"SettingsDownloadLimits", 
"SettingsDownloads", 
"SettingsDownloadsMax", 
"SettingsDownloadsSpeedPause", 
"SettingsExampleText", 
"SettingsFilterMessages", 
"SettingsFinishedDirty", 
"SettingsFullRowSelect", 
"SettingsGeneral", 
"SettingsHashFiles", 
"SettingsHubUserCommands", 
"SettingsIgnoreOffline", 
"SettingsIp", 
"SettingsKeepLists", 
"SettingsLanguageFile", 
"SettingsLogDir", 
"SettingsLogDownloads", 
"SettingsLogFilelistTransfers", 
"SettingsLogMainChat", 
"SettingsLogPrivateChat", 
"SettingsLogUploads", 
"SettingsLogging", 
"SettingsLogs", 
"SettingsMaxTabRows", 
"SettingsMinimizeTray", 
"SettingsName", 
"SettingsNoAwaymsgToBots", 
"SettingsOpenFavoriteHubs", 
"SettingsOpenFinishedDownloads", 
"SettingsOpenPublic", 
"SettingsOpenQueue", 
"SettingsOptions", 
"SettingsPassive", 
"SettingsPersonalInformation", 
"SettingsPmBeep", 
"SettingsPmBeepOpen", 
"SettingsPopupOffline", 
"SettingsPopupPms", 
"SettingsPort", 
"SettingsPublicHubList", 
"SettingsPublicHubListUrl", 
"SettingsPublicHubListHttpProxy", 
"SettingsRemoveDupes", 
"SettingsRequiresRestart", 
"SettingsRollback", 
"SettingsSelectTextFace", 
"SettingsSelectWindowColor", 
"SettingsSfvCheck", 
"SettingsShareHidden", 
"SettingsShareSize", 
"SettingsSharedDirectories", 
"SettingsShowJoins", 
"SettingsShowProgressBars", 
"SettingsSkipZeroByte", 
"SettingsSmallSendBuffer", 
"SettingsSocks5", 
"SettingsSocks5Ip", 
"SettingsSocks5Port", 
"SettingsSocks5Resolve", 
"SettingsSocks5Username", 
"SettingsSounds", 
"SettingsSpeedsNotAccurate", 
"SettingsStatusInChat", 
"SettingsTabCompletion", 
"SettingsTimeStamps", 
"SettingsUnfinishedDownloadDirectory", 
"SettingsUploads", 
"SettingsUploadsMinSpeed", 
"SettingsUploadsSlots", 
"SettingsUrlHandler", 
"SettingsUseOemMonofont", 
"SettingsUseSystemIcons", 
"SettingsUserCommands", 
"SettingsWriteBuffer", 
"SfvInconsistency", 
"Shared", 
"SharedFiles", 
"Size", 
"SizeMax", 
"SizeMin", 
"SlotGranted", 
"Slots", 
"SlotsSet", 
"SocketShutDown", 
"SocksAuthFailed", 
"SocksAuthUnsupported", 
"SocksFailed", 
"SocksNeedsAuth", 
"SocksSetupError", 
"SourceType", 
"SpecifyServer", 
"SpecifySearchString", 
"Speed", 
"Status", 
"StoredPasswordSent", 
"Tag", 
"TargetFilenameTooLong", 
"Tb", 
"Time", 
"TimeLeft", 
"TimestampsDisabled", 
"TimestampsEnabled", 
"Total", 
"TthRoot", 
"Type", 
"UnableToCreateThread", 
"Unknown", 
"UnknownAddress", 
"UnknownError", 
"UnsupportedFilelistFormat", 
"UploadFinishedIdle", 
"UploadStarting", 
"UploadedBytes", 
"UploadedTo", 
"Uploads", 
"User", 
"UserDescription", 
"UserOffline", 
"UserWentOffline", 
"Users", 
"Video", 
"ViewAsText", 
"Waiting", 
"WaitingUserOnline", 
"WaitingUsersOnline", 
"WaitingToRetry", 
"WhatsThis", 
"Yes", 
"YouAreBeingRedirected", 
"CopyToClipboard", 
"AutoUpdateIncoming", 
"ExpandQueue", 
"SearchSites", 
"MenuTransferView", 
"StripIsp", 
"StripIspPm", 
"Isp", 
"HubBoldTabs", 
"PmBoldTabs", 
"QueueBoldTabs", 
"HighPrioSample", 
"RotateLog", 
"PopupAway", 
"PopupOnPm", 
"PopupOnNewPm", 
"PopupOnHubstatus", 
"HubframeConfirmation", 
"QueueRemoveConfirmation", 
"Copy", 
"AddEmpty", 
"BadRegexp", 
"AllUploads", 
"AllDownloads", 
"PopupsActivated", 
"PopupsDeactivated", 
"ShowLog", 
"SettingsFulAdvanced", 
"SettingsFulHighlight", 
"SettingsFulMore", 
"PopunderPm", 
"PopunderDirlist", 
"Url", 
"Key", 
"DialogOk", 
"DialogCancel", 
"SettingsWsTitle", 
"SettingsWsHowto", 
"SettingsWsDescr", 
"NameRequired", 
"NameAlreadyInUse", 
"KeyAlreadyInUse", 
"TabActiveBg", 
"TabActiveText", 
"TabActiveBorder", 
"TabInactiveBg", 
"TabShowIcons", 
"TabInactiveBgDisconnected", 
"TabInactiveText", 
"TabInactiveBorder", 
"TabInactiveBgNotify", 
"FinishedSearching", 
"LastseenError", 
"Removed", 
"DirectoryNotFound", 
"MenuFulpage", 
"MenuCloseAllPm", 
"MenuCloseAllDirList", 
"CustomSound", 
"Muted", 
"Unmuted", 
};
