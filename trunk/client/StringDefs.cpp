#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
"Active", 
"Enabled / Search String", 
"&Add", 
"Add To Favorites", 
"Added", 
"Address already in use", 
"Address not available", 
"Automatic Directory Listing Search", 
"Destination Directory", 
"Discard", 
"Download Matches", 
"Enabled", 
"Full Path", 
"ADLSearch Properties", 
"Search String", 
"Max FileSize", 
"Min FileSize", 
"Search Type", 
"Size Type", 
"All download slots taken", 
"All %d users offline", 
"All 3 users offline", 
"All 4 users offline", 
"All", 
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
"Close", 
"Close connection", 
"Closing connection...", 
"Compressed", 
"Error during compression", 
"&Configure", 
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
"Configured Public Hub Lists", 
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
"Directory or directory name already exists", 
"Disk full(?)", 
"Disconnect user(s)", 
"Disconnected", 
"Disconnected user leaving the hub: ", 
"Document", 
"Done", 
"The temporary download directory cannot be shared", 
"Download", 
"Download failed: ", 
"Download finished, idle...", 
"Download Queue", 
"Download starting...", 
"Download to...", 
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
"&Edit", 
"E-Mail", 
"Please enter a nickname in the settings dialog!", 
"Please enter a password", 
"Please enter a reason", 
"Enter search string", 
"Please enter a destination server", 
"Errors", 
"Exact size", 
"Executable", 
"Join/part of favorite users showing off", 
"Join/part of favorite users showing on", 
"Favorite name", 
"Under what name you see the directory", 
"Favorite hub added", 
"Identification (leave blank for defaults)", 
"Favorite Hub Properties", 
"Favorite Hubs", 
"Favorite user added", 
"Favorite Users", 
"File", 
"Files", 
"File list refresh finished", 
"File list refresh initiated", 
"File not available", 
"File type", 
"A file with a different size already exists in the queue", 
"A file with diffent tth root already exists in the queue", 
"Filename", 
"files left", 
"files/h", 
"F&ilter", 
"Filtered: ", 
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
"Creating file index...", 
"Run in background", 
"Statistics", 
"Please wait while DC++ indexes your files (they won't be shared until they've been indexed)...", 
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
"Join/part showing off", 
"Join/part showing on", 
"Joins: ", 
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
"Match queue", 
"Matched %d file(s)", 
"Max Hubs", 
"Max Users", 
"MiB", 
"MiB/s", 
"About DC++...", 
"ADL Search", 
"Arrange icons", 
"Cascade", 
"Change Log", 
"Close disconnected", 
"Help &Contents\tF1", 
"DC++ discussion forum", 
"Donate ���/$$$ (paypal)", 
"&Download Queue\tCtrl+D", 
"&Exit", 
"Frequently asked questions", 
"&Favorite Hubs\tCtrl+F", 
"Favorite &Users\tCtrl+U", 
"&File", 
"Follow last redirec&t\tCtrl+T", 
"Indexing progress", 
"&Help", 
"Downloads", 
"Help forum", 
"Translations", 
"DC++ Homepage", 
"Horizontal Tile", 
"Minimize &All", 
"Restore All", 
"Network Statistics", 
"&Notepad\tCtrl+N", 
"Open downloads directory", 
"Open file list...\tCtrl+L", 
"Open own list", 
"&Public Hubs\tCtrl+P", 
"&Quick Connect ...\tCtrl+Q", 
"Readme / Newbie help", 
"&Reconnect\tCtrl+R", 
"Refresh file list\tCtrl+E", 
"Report a bug", 
"Request a feature", 
"&Search\tCtrl+S", 
"Search Spy", 
"Settings...", 
"Show", 
"&Status bar\tCtrl+2", 
"&Toolbar\tCtrl+1", 
"T&ransfers\tCtrl+3", 
"Vertical Tile", 
"&View", 
"&Window", 
"Min Share", 
"Min Slots", 
"Move/Rename", 
"Move &Down", 
"Move &Up", 
"Network Statistics", 
"Network unreachable (are you connected to the internet?)", 
"&New...", 
"Next", 
"Nick", 
"Your nick was already taken, please change to something else!", 
" (Nick unknown)", 
"No", 
"No directory specified", 
"You're trying to download from yourself!", 
"No errors", 
"No matches", 
"No slots available", 
"No users", 
"No users to download from", 
"Non-blocking operation still in progress", 
"Normal", 
"Not connected", 
"Not a socket", 
"Notepad", 
"Offline", 
"Online", 
"Only users with free slots", 
"Only results with TTH root", 
"Only where I'm op", 
"Open", 
"Open download page?", 
"Open folder", 
"Operating system does not match minimum requirements for feature", 
"Operation would block execution", 
"Out of buffer space", 
"Parts: ", 
"Passive user", 
"Password", 
"Path", 
"Paused", 
"Permission denied", 
"PiB", 
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
"Really remove?", 
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
"Settings", 
"Add finished files to share instantly (if shared)", 
"&Add folder", 
"Break on first ADLSearch match", 
"DC++\\Advanced", 
"DC++\\Advanced\\Experts only", 
"Advanced settings", 
"Use antifragmentation method for downloads", 
"DC++\\Appearance", 
"DC++\\Colors and sounds", 
"Auto-away on minimize (and back on restore)", 
"Automatically follow redirects", 
"Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", 
"Automatically search for alternative download locations", 
"Automatically match queue for auto search hits", 
"Automatically refresh share", 
"&Change", 
"Clear search box after each search", 
"Colors", 
"Command", 
"Enable safe and compressed transfers", 
"Configure Public Hub Lists", 
"Confirm application exit", 
"Confirm favorite hub removal", 
"Confirm item removal in download queue", 
"Connection Settings (see the help file if unsure)", 
"Connection Type", 
"Default away message", 
"Directories", 
"Don't download files already in share", 
"Default download directory", 
"Limits", 
"DC++\\Downloads", 
"Maximum simultaneous downloads (0 = infinite)", 
"No new downloads if speed exceeds (KiB/s, 0 = disable)", 
"Donate ���:s! (ok, dirty dollars are fine as well =) (see help menu)", 
"Only show joins / parts for favorite users", 
"Downloads\\Download to", 
"Favorite download to directories", 
"Filename", 
"Filter kick and NMDC debug messages", 
"Set Finished Manager(s) tab bold when an entry is added", 
"Format", 
"DC++", 
"Get User Country", 
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
"Log status messages", 
"Log system messages", 
"Log uploads", 
"Logging", 
"DC++\\Logs", 
"Max hash speed", 
"Max tab rows", 
"Minimize to tray", 
"Name", 
"DC++\\Connection settings", 
"Don't send the away message to bots", 
"Note; New files are added to the share only once they've been hashed!", 
"Search for files with TTH root only as standard", 
"Open the favorite hubs window at startup", 
"Open the finished downloads window at startup", 
"Open new window when using /join", 
"Open the public hubs window at startup", 
"Open the download queue window at startup", 
"Always open help file with this dialog", 
"Options", 
"Passive", 
"Personal Information", 
"Make an annoying sound every time a private message is received", 
"Make an annoying sound when a private message window is opened", 
"PM history", 
"Open new file list windows in the background", 
"Open new private message windows in the background", 
"Open private messages from offline users in their own window", 
"Open private messages in their own window", 
"Public Hubs list", 
"HTTP Proxy (for hublist only)", 
"Public Hubs list URL", 
"Set Download Queue tab bold when contents change", 
"Rename", 
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
"TCP Port", 
"Show timestamps in chat by default", 
"UDP Port", 
"Unfinished downloads directory", 
"DC++\\Sharing", 
"Automatically open an extra slot if speed is below (0 = disable)", 
"Upload slots", 
"Register with Windows to handle dchub:// and adc:// URL links", 
"Use CTRL for line history", 
"Use OEM monospaced font for viewing text files", 
"Use system icons when browsing files (slows browsing down a bit)", 
"Use UPnP Control", 
"DC++\\Advanced\\User Commands", 
"Write buffer size", 
"CRC32 inconsistency (SFV-Check)", 
"Shared", 
"Shared Files", 
"Size", 
"Max Size", 
"Min Size", 
"New virtual name matches old name, skipping...", 
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
"Specify a search string", 
"Specify a server to connect to", 
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
"A file with the same hash already exists in your share", 
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
"Chat", 
"Command", 
"Context", 
"Filelist Menu", 
"Hub IP / DNS (empty = all, 'op' = where operator)", 
"Hub Menu", 
"Send once per nick", 
"Parameters", 
"PM", 
"Text sent to hub", 
"Raw", 
"Search Menu", 
"To", 
"Command Type", 
"User Menu", 
"Create / Modify Command", 
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
"Waiting to retry...", 
"Waiting (User online)", 
"Waiting (%d of %d users online)", 
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
"Only show popups if away", 
"Only show popups if minimized to tray", 
"Show popup on PM", 
"Show popup on PM only if it's from a new user", 
"Show popup on hub status messages such as disconnected", 
"Appearance", 
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
"fulDC\\Sharing", 
"fulDC", 
"fulDC\\Appearance\\Highlight", 
"fulDC\\Downloads", 
"fulDC\\Appearance", 
"fulDC\\Appearance\\Tabs", 
"fulDC\\Appearance\\Popups", 
"Automatically refresh incoming", 
"Skiplist", 
"Skiplist (separate files with |):", 
"Skiplist", 
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
"Flash Window", 
"Include Nick", 
"Whole Line", 
"Case Sensitive", 
"Entire Word", 
"Change Tabcolor", 
"Lastlog", 
"Dupes Background Color", 
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
"Resolving", 
"Failed to resolve", 
"Not found in help file", 
"Results: ", 
"Notify Me", 
"%a - Abbreviated weekday name\n%A - Full weekday name\n%b - Abbreviated month name\n%B - Full month name\n%c - Date and time representation appropriate for locale\n%d - Day of month as decimal number (01 � 31)\n%H - Hour in 24-hour format (00 � 23)\n%I - Hour in 12-hour format (01 � 12)\n%j - Day of year as decimal number (001 � 366)\n%m - Month as decimal number (01 � 12)\n%M - Minute as decimal number (00 � 59)\n%p - Current locale's A.M./P.M. indicator for 12-hour clock\n%S - Second as decimal number (00 � 59)\n%U - Week of year as decimal number, with Sunday as first day of week (00 � 53)\n%w - Weekday as decimal number (0 � 6; Sunday is 0)\n%W - Week of year as decimal number, with Monday as first day of week (00 � 53)\n%x - Date representation for current locale\n%X - Time representation for current locale\n%y - Year without century, as decimal number (00 � 99)\n%Y - Year with century, as decimal number\n%z, %Z - Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown\n%% - Percent sign", 
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
"Toggle window when selecting an active tab", 
"Enabled logging of this hub", 
"Disabled logging of this hub", 
"Strip ISP", 
"Log Chat", 
"Name", 
"Don't display popups for active window", 
"Activate window when clicking popup", 
"Drop connections from own nick(stupid clients)", 
"Incoming connection dropped because user tried to use a fake nick, IP=", 
"Flash window on PM", 
"Flash window on PM only if it's from a new user", 
"Ignore TTH inconsistency on downloads", 
};
string ResourceManager::names[] = {
"Active", 
"ActiveSearchString", 
"Add", 
"AddToFavorites", 
"Added", 
"AddressAlreadyInUse", 
"AddressNotAvailable", 
"AdlSearch", 
"AdlsDestination", 
"AdlsDiscard", 
"AdlsDownload", 
"AdlsEnabled", 
"AdlsFullPath", 
"AdlsProperties", 
"AdlsSearchString", 
"AdlsSizeMax", 
"AdlsSizeMin", 
"AdlsType", 
"AdlsUnits", 
"AllDownloadSlotsTaken", 
"AllUsersOffline", 
"All3UsersOffline", 
"All4UsersOffline", 
"All", 
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
"Close", 
"CloseConnection", 
"ClosingConnection", 
"Compressed", 
"CompressionError", 
"Configure", 
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
"ConfiguredHubLists", 
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
"DirectoryAddError", 
"DiscFull", 
"DisconnectUser", 
"Disconnected", 
"DisconnectedUser", 
"Document", 
"Done", 
"DontShareTempDirectory", 
"Download", 
"DownloadFailed", 
"DownloadFinishedIdle", 
"DownloadQueue", 
"DownloadStarting", 
"DownloadTo", 
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
"EditAccel", 
"Email", 
"EnterNick", 
"EnterPassword", 
"EnterReason", 
"EnterSearchString", 
"EnterServer", 
"Errors", 
"ExactSize", 
"Executable", 
"FavJoinShowingOff", 
"FavJoinShowingOn", 
"FavoriteDirName", 
"FavoriteDirNameLong", 
"FavoriteHubAdded", 
"FavoriteHubIdentity", 
"FavoriteHubProperties", 
"FavoriteHubs", 
"FavoriteUserAdded", 
"FavoriteUsers", 
"File", 
"Files", 
"FileListRefreshFinished", 
"FileListRefreshInitiated", 
"FileNotAvailable", 
"FileType", 
"FileWithDifferentSize", 
"FileWithDifferentTth", 
"Filename", 
"FilesLeft", 
"FilesPerHour", 
"Filter", 
"Filtered", 
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
"HashProgress", 
"HashProgressBackground", 
"HashProgressStats", 
"HashProgressText", 
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
"JoinShowingOff", 
"JoinShowingOn", 
"Joins", 
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
"MatchQueue", 
"MatchedFiles", 
"MaxHubs", 
"MaxUsers", 
"Mb", 
"Mbps", 
"MenuAbout", 
"MenuAdlSearch", 
"MenuArrange", 
"MenuCascade", 
"MenuChangelog", 
"MenuCloseDisconnected", 
"MenuContents", 
"MenuDiscuss", 
"MenuDonate", 
"MenuDownloadQueue", 
"MenuExit", 
"MenuFaq", 
"MenuFavoriteHubs", 
"MenuFavoriteUsers", 
"MenuFile", 
"MenuFollowRedirect", 
"MenuHashProgress", 
"MenuHelp", 
"MenuHelpDownloads", 
"MenuHelpForum", 
"MenuHelpTranslations", 
"MenuHomepage", 
"MenuHorizontalTile", 
"MenuMinimizeAll", 
"MenuRestoreAll", 
"MenuNetworkStatistics", 
"MenuNotepad", 
"MenuOpenDownloadsDir", 
"MenuOpenFileList", 
"MenuOpenOwnList", 
"MenuPublicHubs", 
"MenuQuickConnect", 
"MenuReadme", 
"MenuReconnect", 
"MenuRefreshFileList", 
"MenuReportBug", 
"MenuRequestFeature", 
"MenuSearch", 
"MenuSearchSpy", 
"MenuSettings", 
"MenuShow", 
"MenuStatusBar", 
"MenuToolbar", 
"MenuTransferView", 
"MenuVerticalTile", 
"MenuView", 
"MenuWindow", 
"MinShare", 
"MinSlots", 
"Move", 
"MoveDown", 
"MoveUp", 
"NetworkStatistics", 
"NetworkUnreachable", 
"New", 
"Next", 
"Nick", 
"NickTaken", 
"NickUnknown", 
"NoStr", 
"NoDirectorySpecified", 
"NoDownloadsFromSelf", 
"NoErrors", 
"NoMatches", 
"NoSlotsAvailable", 
"NoUsers", 
"NoUsersToDownloadFrom", 
"NonBlockingOperation", 
"Normal", 
"NotConnected", 
"NotSocket", 
"Notepad", 
"Offline", 
"Online", 
"OnlyFreeSlots", 
"OnlyTth", 
"OnlyWhereOp", 
"Open", 
"OpenDownloadPage", 
"OpenFolder", 
"OperatingSystemNotCompatible", 
"OperationWouldBlockExecution", 
"OutOfBufferSpace", 
"Parts", 
"PassiveUser", 
"Password", 
"Path", 
"Paused", 
"PermissionDenied", 
"Pib", 
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
"ReallyRemove", 
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
"Settings", 
"SettingsAddFinishedInstantly", 
"SettingsAddFolder", 
"SettingsAdlsBreakOnFirst", 
"SettingsAdvanced", 
"SettingsAdvanced3", 
"SettingsAdvancedSettings", 
"SettingsAntiFrag", 
"SettingsAppearance", 
"SettingsAppearance2", 
"SettingsAutoAway", 
"SettingsAutoFollow", 
"SettingsAutoKick", 
"SettingsAutoSearch", 
"SettingsAutoSearchAutoMatch", 
"SettingsAutoUpdateList", 
"SettingsChange", 
"SettingsClearSearch", 
"SettingsColors", 
"SettingsCommand", 
"SettingsCompressTransfers", 
"SettingsConfigureHubLists", 
"SettingsConfirmExit", 
"SettingsConfirmHubRemoval", 
"SettingsConfirmItemRemoval", 
"SettingsConnectionSettings", 
"SettingsConnectionType", 
"SettingsDefaultAwayMsg", 
"SettingsDirectories", 
"SettingsDontDlAlreadyShared", 
"SettingsDownloadDirectory", 
"SettingsDownloadLimits", 
"SettingsDownloads", 
"SettingsDownloadsMax", 
"SettingsDownloadsSpeedPause", 
"SettingsExampleText", 
"SettingsFavShowJoins", 
"SettingsFavoriteDirsPage", 
"SettingsFavoriteDirs", 
"SettingsFileName", 
"SettingsFilterMessages", 
"SettingsFinishedDirty", 
"SettingsFormat", 
"SettingsGeneral", 
"SettingsGetUserCountry", 
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
"SettingsLogStatusMessages", 
"SettingsLogSystemMessages", 
"SettingsLogUploads", 
"SettingsLogging", 
"SettingsLogs", 
"SettingsMaxHashSpeed", 
"SettingsMaxTabRows", 
"SettingsMinimizeTray", 
"SettingsName", 
"SettingsNetwork", 
"SettingsNoAwaymsgToBots", 
"SettingsOnlyHashed", 
"SettingsOnlyTth", 
"SettingsOpenFavoriteHubs", 
"SettingsOpenFinishedDownloads", 
"SettingsOpenNewWindow", 
"SettingsOpenPublic", 
"SettingsOpenQueue", 
"SettingsOpenUserCmdHelp", 
"SettingsOptions", 
"SettingsPassive", 
"SettingsPersonalInformation", 
"SettingsPmBeep", 
"SettingsPmBeepOpen", 
"SettingsPmHistory", 
"SettingsPopunderFilelist", 
"SettingsPopunderPm", 
"SettingsPopupOffline", 
"SettingsPopupPms", 
"SettingsPublicHubList", 
"SettingsPublicHubListHttpProxy", 
"SettingsPublicHubListUrl", 
"SettingsQueueDirty", 
"SettingsRenameFolder", 
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
"SettingsTcpPort", 
"SettingsTimeStamps", 
"SettingsUdpPort", 
"SettingsUnfinishedDownloadDirectory", 
"SettingsUploads", 
"SettingsUploadsMinSpeed", 
"SettingsUploadsSlots", 
"SettingsUrlHandler", 
"SettingsUseCtrlForLineHistory", 
"SettingsUseOemMonofont", 
"SettingsUseSystemIcons", 
"SettingsUseUpnp", 
"SettingsUserCommands", 
"SettingsWriteBuffer", 
"SfvInconsistency", 
"Shared", 
"SharedFiles", 
"Size", 
"SizeMax", 
"SizeMin", 
"SkipRename", 
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
"SpecifySearchString", 
"SpecifyServer", 
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
"TthAlreadyShared", 
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
"UserCmdChat", 
"UserCmdCommand", 
"UserCmdContext", 
"UserCmdFilelistMenu", 
"UserCmdHub", 
"UserCmdHubMenu", 
"UserCmdOnce", 
"UserCmdParameters", 
"UserCmdPm", 
"UserCmdPreview", 
"UserCmdRaw", 
"UserCmdSearchMenu", 
"UserCmdTo", 
"UserCmdType", 
"UserCmdUserMenu", 
"UserCmdWindow", 
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
"WaitingToRetry", 
"WaitingUserOnline", 
"WaitingUsersOnline", 
"YesStr", 
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
"PopupAway", 
"PopupMinimized", 
"PopupOnPm", 
"PopupOnNewPm", 
"PopupOnHubstatus", 
"PopupAppearance", 
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
"SettingsFulPopup", 
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
"SettingsFlashWindow", 
"SettingsIncludeNick", 
"SettingsWholeLine", 
"SettingsCaseSensitive", 
"SettingsEntireWord", 
"SettingsTabColor", 
"SettingsLastlog", 
"SettingsDupes", 
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
"Resolving", 
"FailedResolve", 
"NotFound", 
"Results", 
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
"ToggleActiveWindow", 
"LoggingEnabled", 
"LoggingDisabled", 
"FhStripIsp", 
"FhLogChat", 
"FolderName", 
"PopupDontShowOnActive", 
"PopupActivateOnClick", 
"DropStupidConnection", 
"DropStupidConnectionLog", 
"FlashWindowOnPm", 
"FlashWindowOnNewPm", 
"IgnoreTthInconsistency", 
};
