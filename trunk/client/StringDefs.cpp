#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
"Active", 
"Enabled / Search String", 
"&Add", 
"Added", 
"Add finished files to share instantly (if shared)", 
"Use UPnP Control", 
"Add To Favorites", 
"Address already in use", 
"Address not available", 
"Discard", 
"Automatic Directory Listing Search", 
"Full Path", 
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
"Ban user(s)", 
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
"Copy address to clipboard", 
"Copy magnet link to clipboard", 
"Copy nick to clipboard", 
"Could not open target file: ", 
"Count", 
"Country", 
"CRC Checked", 
"Error during decompression", 
"Description", 
"Destination", 
"Directory", 
"Directory already shared", 
"Disconnected", 
"Disconnected user leaving the hub: ", 
"Disconnect user(s)", 
"Disk full(?)", 
"Document", 
"Done", 
"The temporary download directory cannot be shared", 
"Download", 
"Download failed: ", 
"Download finished, idle...", 
"Download starting...", 
"Download to...", 
"Download Queue", 
"Download whole directory", 
"Download whole directory to...", 
"Downloaded", 
"Downloaded %s (%.01f%%) in %s", 
" downloaded from ", 
"Downloading...", 
"Downloading public hub list...", 
"Downloads", 
"Duplicate file will not be shared: ", 
"Dupe matched against: ", 
"Duplicate source", 
"Edit", 
"E-Mail", 
"Please enter a nickname in the settings dialog!", 
"Please enter a password", 
"Please enter a reason", 
"Enter search string", 
"Please enter a destination server", 
"Errors", 
"Exact size", 
"Executable", 
"Favorite Hubs", 
"Favorite hub added", 
"Favorite Users", 
"Favorite user added", 
"Join/part of favorite users showing off", 
"Join/part of favorite users showing on", 
"File", 
"Files", 
"File list refresh finished", 
"File list refresh initiated", 
"File not available", 
"File type", 
"A file with a different size already exists in the queue", 
"A file with diffent tth root already exists in the queue", 
"Filename", 
"files/h", 
"files left", 
"F&ilter", 
"Find", 
"Finished Downloads", 
"Finished Uploads", 
"File with '$' cannot be downloaded and will not be shared: ", 
"Force attempt", 
"GiB", 
"Get file list", 
"Go to directory", 
"Grant extra slot", 
"Hash database", 
"Hash database rebuilt", 
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
"Downloaded tree does not match TTH root", 
"IP: ", 
"IP", 
"Items", 
"Joins: ", 
"Join/part showing off", 
"Join/part showing on", 
"KiB", 
"KiB/s", 
"Kick user(s)", 
"A file of equal or larger size already exists at the target location", 
"Last change: ", 
"Hub (last seen on if offline)", 
"Time last seen", 
"left", 
"Loading DC++, please wait...", 
"Lookup TTH at Bitzi.com", 
"Low", 
"Lowest", 
"Manual connect address", 
"Match queue", 
"Matched %d file(s)", 
"Max Hubs", 
"Max Users", 
"MiB", 
"MiB/s", 
"&File", 
"ADL Search", 
"&Download Queue\tCtrl+D", 
"&Exit", 
"&Favorite Hubs\tCtrl+F", 
"Favorite &Users\tCtrl+U", 
"Follow last redirec&t\tCtrl+T", 
"Indexing progress", 
"Network Statistics", 
"&Notepad\tCtrl+N", 
"Open file list...\tCtrl+L", 
"&Public Hubs\tCtrl+P", 
"&Quick Connect ...\tCtrl+Q", 
"&Reconnect\tCtrl+R", 
"Refresh file list", 
"Show", 
"&Search\tCtrl+S", 
"Search Spy", 
"Settings...\tCtrl+E", 
"&Help", 
"About DC++...", 
"Change Log", 
"Help &Contents\tF1", 
"DC++ discussion forum", 
"Donate €€€/$$$ (paypal)", 
"Downloads", 
"Translations", 
"Frequently asked questions", 
"Help forum", 
"DC++ Homepage", 
"Open downloads directory", 
"Readme / Newbie help", 
"Request a feature", 
"Report a bug", 
"&View", 
"&Status bar\tCtrl+2", 
"&Toolbar\tCtrl+1", 
"T&ransfers\tCtrl+3", 
"&Window", 
"Arrange icons", 
"Cascade", 
"Horizontal Tile", 
"Vertical Tile", 
"Close disconnected", 
"Minimize &All", 
"Min Share", 
"Min Slots", 
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
"Operating system does not match minimum requirements for feature", 
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
"Quick Connect", 
"Rating", 
"Ratio", 
"Re-add source", 
"Really exit?", 
"Redirect", 
"Redirect request received to a hub that's already connected", 
"Redirect user(s)", 
"&Refresh", 
"Refresh user list", 
"Reliability", 
"&Remove", 
"Remove all", 
"Remove all subdirectories before adding this one", 
"Remove user from queue", 
"Remove source", 
"Rollback inconsistency, existing file does not match the one being downloaded", 
"Running...", 
"Search", 
"Search by TTH", 
"Search for", 
"Search for alternates", 
"Search for file", 
"Search options", 
"Search spam detected from ", 
"Search Spy", 
"Search String", 
"Searching for ", 
"Request to seek beyond the end of data", 
"Send private message", 
"Separator", 
"Server", 
"Set priority", 
"&Add folder", 
"Break on first ADLSearch match", 
"DC++\\Advanced", 
"Advanced settings", 
"Use antifragmentation method for downloads", 
"DC++\\Appearance", 
"Auto-away on minimize (and back on restore)", 
"Automatically follow redirects", 
"Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", 
"Automatically search for alternative download locations", 
"Automatically match queue for auto search hits", 
"Automatically refresh share", 
"&Change", 
"Clear search box after each search", 
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
"DC++\\Downloads", 
"Maximum simultaneous downloads (0 = infinite)", 
"No new downloads if speed exceeds (KiB/s, 0 = disable)", 
"Donate €€€:s! (ok, dirty dollars are fine as well =) (see help menu)", 
"Only show joins / parts for favorite users", 
"Filter kick and NMDC debug messages", 
"Set Finished Manager(s) tab bold when an entry is added", 
"Format", 
"DC++\\General", 
"Accept custom user commands from hub", 
"Ignore private messages from offline users", 
"IP", 
"Don't delete file lists when exiting", 
"Language file", 
"Keep duplicate files in your file list (duplicates never count towards your share size)", 
"Log downloads", 
"Log filelist transfers", 
"Log main chat", 
"Log private chat", 
"Log uploads", 
"Log system messages", 
"Logging", 
"DC++\\Logs and Sound", 
"Max hash speed", 
"Max tab rows", 
"Minimize to tray", 
"Name", 
"Don't send the away message to bots", 
"Note; New files are added to the share only once they've been hashed!", 
"Open the favorite hubs window at startup", 
"Open the finished downloads window at startup", 
"Open the public hubs window at startup", 
"Open the download queue window at startup", 
"Options", 
"Passive", 
"Personal Information", 
"Make an annoying sound every time a private message is received", 
"Make an annoying sound when a private message window is opened", 
"Open new file list windows in the background", 
"Open new private message windows in the background", 
"Open private messages from offline users in their own window", 
"Open private messages in their own window", 
"Port (empty=random)", 
"Public Hubs list", 
"Public Hubs list URL", 
"HTTP Proxy (for hublist only)", 
"Set Download Queue tab bold when contents change", 
"Note; most of these options require that you restart DC++", 
"Rollback", 
"Select &text style", 
"Select &window color", 
"Send unknown /commands to the hub", 
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
"Unfinished downloads directory", 
"DC++\\Sharing", 
"Automatically open an extra slot if speed is below (0 = disable)", 
"Upload slots", 
"Register with Windows to handle dchub:// URL links", 
"Use OEM monospaced font for viewing text files", 
"Use system icons when browsing files (slows browsing down a bit)", 
"DC++\\Advanced\\User Commands", 
"Write buffer size", 
"Get User Country", 
"Log status messages", 
"Settings", 
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
"TiB", 
"Time", 
"Time left", 
"Timestamps disabled", 
"Timestamps enabled", 
"More data was sent than was expected", 
"Total: ", 
"TTH inconsistency", 
"TTH Root", 
"Type", 
"Unable to create thread", 
"Unknown", 
"Unknown address", 
"Unknown command: ", 
"Unknown error: 0x%x", 
"Unsupported filelist format", 
"Upload finished, idle...", 
"Upload starting...", 
"Uploaded %s (%.01f%%) in %s", 
" uploaded to ", 
"Uploads", 
"Failed to create port mappings", 
"Failed to remove port mappings", 
"User", 
"User Description", 
"User offline", 
"User went offline", 
"Users", 
"Video", 
"View as text", 
"Virtual name", 
"Virtual directory name already exists", 
"Name under which the others see the directory", 
"Waiting...", 
"Waiting (User online)", 
"Waiting (%d of %d users online)", 
"Waiting to retry...", 
"What's &this?", 
"Yes", 
"You are being redirected to ", 
"Copy to clipboard", 
"Expand Queue", 
"Search Sites", 
"Strip ISP (public hubs only, see favorite hubs settings)", 
"Strip ISP in private messages", 
"ISP", 
"Make tabs bold on status messages in Hubs", 
"Make tabs bold on status messages in private messages", 
"Set high priority on sample and sub files when enqueued", 
"Create new main chat log every midnight", 
"Only show popups if away", 
"Only show popups if minimized to tray", 
"Show popup on PM", 
"Show popup on PM only if it's from a new user", 
"Show popup on hub status messages such as disconnected", 
"Confirm hub exit", 
"Confirm removal of files from download queue", 
"Confirm removal of favorites hub entries", 
"Copy", 
"Can't add an empty string", 
"Badly formatted regular expression", 
"All Uploads", 
"All Downloads", 
"Popups turned on", 
"Popups turned off", 
"View Log", 
"Open Log Directory", 
"fulDC\\Share", 
"fulDC\\Advanced", 
"fulDC\\Appearance\\Highlight", 
"fulDC\\Download", 
"fulDC\\Appearance", 
"fulDC\\Appearance\\Tabs", 
"Automatically refresh incoming", 
"Skiplist Downloads", 
"Skiplist (separate files with |):", 
"Skiplist Share", 
"Skiplist (separate files with |):", 
"Refresh options", 
"Incoming", 
"Share", 
"Both values are time in minutes between refreshes", 
"Mainchat buffer size", 
"Max size ~2.1 billion (2^31)", 
"Tab Colors", 
"Popup", 
"Size of string to display in tabs", 
"Popup time in seconds", 
"Max message length", 
"Select Color", 
"Font", 
"Text Color", 
"Minislots", 
"Static \"download to\" directories", 
"Max sources for match queue", 
"Minislots file extensions", 
"Minislots file size in KiB", 
"Separate paths with |", 
"Web Shortcuts", 
"Highlight Settings", 
"Settings", 
"Match Type", 
"Update", 
"Move Up", 
"Move Down", 
"Background Color", 
"Select Sound File", 
"Display Popup", 
"Play Sound", 
"Include Nick", 
"Whole Line", 
"Case Sensitive", 
"Entire Word", 
"Change Tabcolor", 
"Lastlog", 
"Bold", 
"Italic", 
"Underline", 
"Strikeout", 
"Open private messages in background", 
"Open file lists in background", 
"Url", 
"Key", 
"&OK", 
"&Cancel", 
"Web shortcuts properties", 
"How to use", 
"Web shortcuts are used to open web pages with specific contents. The key can be more than one character, but must not contain any space. The URL has to have exactly One \"%s\" which will be replaced with the string you'll enter. If no \"%s\" exists, nothing will be pasted to the url, and the page will just open up. An example is:\nhttp://domain.com/search=%s\n\nNote: The key is case sensitive!\n\nClean search string means that things like year, xvid, dvdrip, dots and other meaningless info will be removed from the string. e.g \"Movie.Name.2003.DVDRip.XviD-ABC\" will become \"Movie Name\"", 
"Clean search string", 
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
"Finished searching", 
"User hasn't been seen", 
"Removed", 
"Directory not shared", 
"fulDC Homepage", 
"Close all PM windows", 
"Close all file list windows", 
"Close all search windows", 
"Play custom sound on PM's", 
"Muted", 
"Unmuted", 
"Automatically close popup windows", 
"Remove topic (everywhere except hub windowtitle)", 
"Total Time Left", 
"Open my file list", 
"Resolve IP", 
"resolves to", 
"Not found in help file", 
"Results: ", 
"Filtered: ", 
"Notify Me", 
"%a - Abbreviated weekday name\n%A - Full weekday name\n%b - Abbreviated month name\n%B - Full month name\n%c - Date and time representation appropriate for locale\n%d - Day of month as decimal number (01 – 31)\n%H - Hour in 24-hour format (00 – 23)\n%I - Hour in 12-hour format (01 – 12)\n%j - Day of year as decimal number (001 – 366)\n%m - Month as decimal number (01 – 12)\n%M - Minute as decimal number (00 – 59)\n%p - Current locale's A.M./P.M. indicator for 12-hour clock\n%S - Second as decimal number (00 – 59)\n%U - Week of year as decimal number, with Sunday as first day of week (00 – 53)\n%w - Weekday as decimal number (0 – 6; Sunday is 0)\n%W - Week of year as decimal number, with Monday as first day of week (00 – 53)\n%x - Date representation for current locale\n%X - Time representation for current locale\n%y - Year without century, as decimal number (00 – 99)\n%Y - Year with century, as decimal number\n%z, %Z - Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown\n%% - Percent sign", 
"Time Format", 
"Help", 
"Time Format", 
"Blending", 
"User came online", 
"Begins,Contains,Ends,Equals", 
"Do you really want to remove this item?", 
"Do you really want to close this hub?", 
"String to match", 
"New PM from:", 
"Msg:\r\n", 
"says:\r\n", 
"You've been disconnected from:\r\n", 
"Download complete:\r\n", 
"Current number of open slots:", 
"Current number of download slots:", 
"Download slots set", 
"Invalid number of slots", 
"fulDC uptime:", 
"System uptime:", 
"Name already exists", 
"Name to be displayed in menus", 
"Blend tabs instead of using bold font", 
"file left", 
"Changed priority for", 
"Paste", 
};
string ResourceManager::names[] = {
"Active", 
"ActiveSearchString", 
"Add", 
"Added", 
"AddFinishedInstantly", 
"SettingsUseUpnp", 
"AddToFavorites", 
"AddressAlreadyInUse", 
"AddressNotAvailable", 
"AdlDiscard", 
"AdlSearch", 
"AdlFullPath", 
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
"BanUser", 
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
"CopyHub", 
"CopyMagnet", 
"CopyNick", 
"CouldNotOpenTargetFile", 
"Count", 
"Country", 
"CrcChecked", 
"DecompressionError", 
"Description", 
"Destination", 
"Directory", 
"DirectoryAlreadyShared", 
"Disconnected", 
"DisconnectedUser", 
"DisconnectUser", 
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
"DuplicateFileNotShared", 
"DuplicateMatch", 
"DuplicateSource", 
"Edit", 
"Email", 
"EnterNick", 
"EnterPassword", 
"EnterReason", 
"EnterSearchString", 
"EnterServer", 
"Errors", 
"ExactSize", 
"Executable", 
"FavoriteHubs", 
"FavoriteHubAdded", 
"FavoriteUsers", 
"FavoriteUserAdded", 
"FavJoinShowingOff", 
"FavJoinShowingOn", 
"File", 
"Files", 
"FileListRefreshFinished", 
"FileListRefreshInitiated", 
"FileNotAvailable", 
"FileType", 
"FileWithDifferentSize", 
"FileWithDifferentTth", 
"Filename", 
"FilesPerHour", 
"FilesLeft", 
"Filter", 
"Find", 
"FinishedDownloads", 
"FinishedUploads", 
"ForbiddenDollarFile", 
"ForceAttempt", 
"Gb", 
"GetFileList", 
"GoToDirectory", 
"GrantExtraSlot", 
"HashDatabase", 
"HashRebuilt", 
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
"InvalidTree", 
"Ip", 
"IpBare", 
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
"Left", 
"Loading", 
"LookupAtBitzi", 
"Low", 
"Lowest", 
"ManualAddress", 
"MatchQueue", 
"MatchedFiles", 
"MaxHubs", 
"MaxUsers", 
"Mb", 
"Mbps", 
"MenuFile", 
"MenuAdlSearch", 
"MenuDownloadQueue", 
"MenuExit", 
"MenuFavoriteHubs", 
"MenuFavoriteUsers", 
"MenuFollowRedirect", 
"MenuHashProgress", 
"MenuNetworkStatistics", 
"MenuNotepad", 
"MenuOpenFileList", 
"MenuPublicHubs", 
"MenuQuickConnect", 
"MenuReconnect", 
"MenuRefreshFileList", 
"MenuShow", 
"MenuSearch", 
"MenuSearchSpy", 
"MenuSettings", 
"MenuHelp", 
"MenuAbout", 
"MenuChangelog", 
"MenuContents", 
"MenuDiscuss", 
"MenuDonate", 
"MenuHelpDownloads", 
"MenuHelpTranslations", 
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
"MenuTransferView", 
"MenuWindow", 
"MenuArrange", 
"MenuCascade", 
"MenuHorizontalTile", 
"MenuVerticalTile", 
"MenuCloseDisconnected", 
"MenuMinimizeAll", 
"MinShare", 
"MinSlots", 
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
"OperatingSystemNotCompatible", 
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
"QuickConnect", 
"Rating", 
"Ratio", 
"ReaddSource", 
"ReallyExit", 
"Redirect", 
"RedirectAlreadyConnected", 
"RedirectUser", 
"Refresh", 
"RefreshUserList", 
"Reliability", 
"Remove", 
"RemoveAll", 
"RemoveAllSubdirectories", 
"RemoveFromAll", 
"RemoveSource", 
"RollbackInconsistency", 
"Running", 
"Search", 
"SearchByTth", 
"SearchFor", 
"SearchForAlternates", 
"SearchForFile", 
"SearchOptions", 
"SearchSpamFrom", 
"SearchSpy", 
"SearchString", 
"SearchingFor", 
"SeekBeyondEnd", 
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
"SettingsAutoSearchAutoMatch", 
"SettingsAutoUpdateList", 
"SettingsChange", 
"SettingsClearSearch", 
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
"SettingsFavShowJoins", 
"SettingsFilterMessages", 
"SettingsFinishedDirty", 
"SettingsFormat", 
"SettingsGeneral", 
"SettingsHubUserCommands", 
"SettingsIgnoreOffline", 
"SettingsIp", 
"SettingsKeepLists", 
"SettingsLanguageFile", 
"SettingsListDupes", 
"SettingsLogDownloads", 
"SettingsLogFilelistTransfers", 
"SettingsLogMainChat", 
"SettingsLogPrivateChat", 
"SettingsLogUploads", 
"SettingsLogSystemMessages", 
"SettingsLogging", 
"SettingsLogs", 
"SettingsMaxHashSpeed", 
"SettingsMaxTabRows", 
"SettingsMinimizeTray", 
"SettingsName", 
"SettingsNoAwaymsgToBots", 
"SettingsOnlyHashed", 
"SettingsOpenFavoriteHubs", 
"SettingsOpenFinishedDownloads", 
"SettingsOpenPublic", 
"SettingsOpenQueue", 
"SettingsOptions", 
"SettingsPassive", 
"SettingsPersonalInformation", 
"SettingsPmBeep", 
"SettingsPmBeepOpen", 
"SettingsPopunderFilelist", 
"SettingsPopunderPm", 
"SettingsPopupOffline", 
"SettingsPopupPms", 
"SettingsPort", 
"SettingsPublicHubList", 
"SettingsPublicHubListUrl", 
"SettingsPublicHubListHttpProxy", 
"SettingsQueueDirty", 
"SettingsRequiresRestart", 
"SettingsRollback", 
"SettingsSelectTextFace", 
"SettingsSelectWindowColor", 
"SettingsSendUnknownCommands", 
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
"SettingsGetUserCountry", 
"SettingsLogStatusMessages", 
"Settings", 
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
"TooMuchData", 
"Total", 
"TthInconsistency", 
"TthRoot", 
"Type", 
"UnableToCreateThread", 
"Unknown", 
"UnknownAddress", 
"UnknownCommand", 
"UnknownError", 
"UnsupportedFilelistFormat", 
"UploadFinishedIdle", 
"UploadStarting", 
"UploadedBytes", 
"UploadedTo", 
"Uploads", 
"UpnpFailedToCreateMappings", 
"UpnpFailedToRemoveMappings", 
"User", 
"UserDescription", 
"UserOffline", 
"UserWentOffline", 
"Users", 
"Video", 
"ViewAsText", 
"VirtualName", 
"VirtualNameExists", 
"VirtualNameLong", 
"Waiting", 
"WaitingUserOnline", 
"WaitingUsersOnline", 
"WaitingToRetry", 
"WhatsThis", 
"Yes", 
"YouAreBeingRedirected", 
"CopyToClipboard", 
"ExpandQueue", 
"SearchSites", 
"StripIsp", 
"StripIspPm", 
"Isp", 
"HubBoldTabs", 
"PmBoldTabs", 
"HighPrioSample", 
"RotateLog", 
"PopupAway", 
"PopupMinimized", 
"PopupOnPm", 
"PopupOnNewPm", 
"PopupOnHubstatus", 
"HubframeConfirmation", 
"QueueRemoveConfirmation", 
"FavoritesRemoveConfirmation", 
"Copy", 
"AddEmpty", 
"BadRegexp", 
"AllUploads", 
"AllDownloads", 
"PopupsActivated", 
"PopupsDeactivated", 
"ShowLog", 
"OpenLogDir", 
"SettingsFulShare", 
"SettingsFulAdvanced", 
"SettingsFulHighlight", 
"SettingsFulDownload", 
"SettingsFulAppearance", 
"SettingsFulTabs", 
"SettingsAutoUpdateIncoming", 
"SettingsSbSkiplistDownload", 
"SettingsStSkiplistDownload", 
"SettingsSbSkiplistShare", 
"SettingsStSkiplistShare", 
"SettingsSbRefresh", 
"SettingsStRefreshIncoming", 
"SettingsStRefreshShare", 
"SettingsStRefreshHelp", 
"SettingsSbChatbuffersize", 
"SettingsStChatbuffersize", 
"SettingsSbTabColors", 
"SettingsSbPopup", 
"SettingsSbTabSize", 
"SettingsStDisplaytime", 
"SettingsStMessageLength", 
"SettingsBtnColor", 
"SettingsBtnFont", 
"SettingsBtnTextcolor", 
"SettingsSbMinislots", 
"SettingsSbPaths", 
"SettingsSbMaxSources", 
"SettingsStMinislotsExt", 
"SettingsStMinislotsSize", 
"SettingsStPaths", 
"SettingsSbWebShortcuts", 
"SettingsSbHighlight", 
"SettingsSbSettings", 
"SettingsStMatchType", 
"SettingsBtnUpdate", 
"SettingsBtnMoveup", 
"SettingsBtnMovedown", 
"SettingsBtnBgcolor", 
"SettingsSelectSound", 
"SettingsPopup", 
"SettingsPlaySound", 
"SettingsIncludeNick", 
"SettingsWholeLine", 
"SettingsCaseSensitive", 
"SettingsEntireWord", 
"SettingsTabColor", 
"SettingsLastlog", 
"Bold", 
"Italic", 
"Underline", 
"Strikeout", 
"PopunderPm", 
"PopunderDirlist", 
"Url", 
"Key", 
"DialogOk", 
"DialogCancel", 
"SettingsWsTitle", 
"SettingsWsHowto", 
"SettingsWsDescr", 
"SettingsWsClean", 
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
"MenuCloseAllSearchframe", 
"CustomSound", 
"Muted", 
"Unmuted", 
"RemovePopups", 
"RemoveTopic", 
"TotalTimeLeft", 
"MenuOpenMyList", 
"ResolveIp", 
"ResolvesTo", 
"NotFound", 
"Results", 
"Filtered", 
"NotifyMe", 
"HelpTimeStamps", 
"TimeStampsHelpCaption", 
"SettingsTimeStampHelp", 
"SettingsSbTimeStamps", 
"SettingsSbDirtyBlend", 
"UserCameOnline", 
"HighlightMatchTypes", 
"ConfirmRemove", 
"ConfirmClose", 
"HighlightlistHeader", 
"PopupNewPm", 
"PopupMsg", 
"PopupSays", 
"PopupDisconnected", 
"PopupDownloadComplete", 
"CurrentSlots", 
"CurrentDslots", 
"DslotsSet", 
"InvalidSlots", 
"FuldcUptime", 
"SystemUptime", 
"NameExists", 
"DisplayName", 
"BlendTabs", 
"FileLeft", 
"ChangedPriority", 
"Paste", 
};
