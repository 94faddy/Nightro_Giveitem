// ===== ENHANCED ITEMGIVER V2.5 WITH ADVANCED TERRITORY VALIDATION AND NOTIFICATION SYSTEM (COMPLETE) =====

class AttachmentInfo
{
    string classname;
    int quantity;
    string status;
    ref array<ref AttachmentInfo> attachments;
    
    void AttachmentInfo()
    {
        attachments = new array<ref AttachmentInfo>;
        status = "pending";
    }
}

class ItemInfo
{
    string classname;
    int quantity;
    string status;
    string processed_at;
    string error_message;
    int retry_count;
    string last_retry_at;
    ref array<ref AttachmentInfo> attachments;
    
    void ItemInfo()
    {
        attachments = new array<ref AttachmentInfo>;
        status = "pending";
        processed_at = "";
        error_message = "";
        retry_count = 0;
        last_retry_at = "";
    }
}

class ProcessedItemInfo
{
    string classname;
    int quantity;
    string status;
    string processed_at;
    string error_message;
    ref array<ref AttachmentInfo> attachments;
    
    void ProcessedItemInfo()
    {
        attachments = new array<ref AttachmentInfo>;
        status = "delivered";
        processed_at = "";
        error_message = "";
    }
}

class PlayerStateInfo
{
    string steam_id;
    float last_connect_time;
    float last_respawn_time;
    bool is_ready_for_items;
    bool was_dead_on_connect;
    int connect_count;
    float last_notification_time;
    string last_notification_type;
    
    void PlayerStateInfo()
    {
        steam_id = "";
        last_connect_time = 0;
        last_respawn_time = 0;
        is_ready_for_items = false;
        was_dead_on_connect = false;
        connect_count = 0;
        last_notification_time = 0;
        last_notification_type = "";
    }
}

class ItemQueue
{
    string player_name;
    string steam_id;
    string last_updated;
    ref array<ref ItemInfo> items_to_give;
    ref array<ref ProcessedItemInfo> processed_items;
    ref PlayerStateInfo player_state;
    
    void ItemQueue() 
    { 
        items_to_give = new array<ref ItemInfo>;
        processed_items = new array<ref ProcessedItemInfo>;
        player_state = new PlayerStateInfo();
        player_name = "";
        steam_id = "";
        last_updated = "";
    }
}

class NotificationSettings
{
    string message_template;
    int display_duration_seconds;
    int initial_delay_seconds;
    int respawn_delay_seconds;
    int max_retry_attempts;
    int retry_interval_seconds;
    bool require_territory_check;
    
    // Enhanced notification settings
    int notification_interval_seconds;
    bool enable_periodic_notifications;
    
    // Notification messages for different scenarios
    string no_group_message;
    string no_plotpole_message;
    string outside_territory_message;
    string inventory_full_message;
    string item_delivered_message;
    string waiting_conditions_message;
    string max_retry_reached_message;
    
    // Notification intervals for each type (in seconds)
    int no_group_notification_interval;
    int no_plotpole_notification_interval;
    int outside_territory_notification_interval;
    int inventory_full_notification_interval;
    int waiting_conditions_notification_interval;

    void NotificationSettings()
    {
        message_template = "[NIGHTRO SHOP] You received: {QUANTITY}x {CLASS_NAME}";
        display_duration_seconds = 8;
        initial_delay_seconds = 120;
        respawn_delay_seconds = 300;
        max_retry_attempts = 3;
        retry_interval_seconds = 60;
        require_territory_check = true;
        
        // Enhanced notification settings
        notification_interval_seconds = 60; // Default 1 minute
        enable_periodic_notifications = true;
        
        // Default notification messages
        no_group_message = "[NIGHTRO SHOP] You need to join or create a group to receive items!";
        no_plotpole_message = "[NIGHTRO SHOP] Your group needs to place a Plotpole to receive items! Create your territory first.";
        outside_territory_message = "[NIGHTRO SHOP] You are outside your group's territory! Move closer to your Plotpole. Distance: {DISTANCE}m";
        inventory_full_message = "[NIGHTRO SHOP] Your inventory is full! Clear space to receive {QUANTITY}x {ITEM_NAME}";
        item_delivered_message = "[NIGHTRO SHOP] Items delivered successfully: {QUANTITY}x {ITEM_NAME}";
        waiting_conditions_message = "[NIGHTRO SHOP] Items are waiting for you. Please fulfill the requirements to receive them.";
        max_retry_reached_message = "[NIGHTRO SHOP] Maximum retry attempts reached for {QUANTITY}x {ITEM_NAME}. Item has been removed.";
        
        // Default notification intervals (in seconds)
        no_group_notification_interval = 60; // 1 minute
        no_plotpole_notification_interval = 60; // 1 minute
        outside_territory_notification_interval = 60; // 1 minute
        inventory_full_notification_interval = 60; // 1 minute
        waiting_conditions_notification_interval = 300; // 5 minutes
    }
}

class NightroItemGiverManager
{
    private static const string IG_PROFILE_FOLDER = "$profile:NightroItemGiverData/";
    private static const string IG_PENDING_FOLDER = "pending_items/";
    private static const string IG_SETTINGS_FILE = "notification_settings.json";
    private static const int IG_CHECK_INTERVAL = 60000;
    private static bool m_IsInitialized = false;
    private static ref NotificationSettings m_NotificationSettings;
    private static ref map<string, ref PlayerStateInfo> m_PlayerStates;

    void NightroItemGiverManager()
    {
        Print("===== ENHANCED ITEMGIVER V2.5 WITH ADVANCED TERRITORY VALIDATION AND NOTIFICATION SYSTEM =====");
        Print("[NightroItemGiver] Enhanced Manager with Advanced Notification System created!");
        Init();
    }
    
    static void Init()
    {
        if (m_IsInitialized) return;
            
        Print("[NightroItemGiver] Initializing ENHANCED system with advanced notifications...");
        
        if (!FileExist(IG_PROFILE_FOLDER)) 
        {
            MakeDirectory(IG_PROFILE_FOLDER);
            Print("[NightroItemGiver] Created main folder: " + IG_PROFILE_FOLDER);
        }
        
        if (!FileExist(IG_PROFILE_FOLDER + IG_PENDING_FOLDER)) 
        {
            MakeDirectory(IG_PROFILE_FOLDER + IG_PENDING_FOLDER);
            Print("[NightroItemGiver] Created pending folder: " + IG_PROFILE_FOLDER + IG_PENDING_FOLDER);
        }

        m_PlayerStates = new map<string, ref PlayerStateInfo>;

        LoadNotificationSettings();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckAllOnlinePlayers, IG_CHECK_INTERVAL, true);
        
        m_IsInitialized = true;
        Print("[NightroItemGiver] ENHANCED system with advanced notifications initialized successfully!");
        Print("[NightroItemGiver] Territory Check Enabled: " + m_NotificationSettings.require_territory_check);
        Print("[NightroItemGiver] Periodic Notifications: " + m_NotificationSettings.enable_periodic_notifications);
        Print("[NightroItemGiver] Notification Interval: " + m_NotificationSettings.notification_interval_seconds + "s");
    }
    
    static void LoadNotificationSettings()
    {
        string settingsPath = IG_PROFILE_FOLDER + IG_SETTINGS_FILE;
        if (FileExist(settingsPath))
        {
            JsonFileLoader<NotificationSettings>.JsonLoadFile(settingsPath, m_NotificationSettings);
            Print("[NightroItemGiver] Notification settings loaded from file");
        }
        else
        {
            m_NotificationSettings = new NotificationSettings();
            JsonFileLoader<NotificationSettings>.JsonSaveFile(settingsPath, m_NotificationSettings);
            Print("[NightroItemGiver] Default notification settings created and saved");
        }
    }

    // ===== NEW: CHECK IF PLAYER HAS PENDING ITEMS =====
    static bool HasPendingItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return false;
        
        string steamID = player.GetIdentity().GetPlainId();
        string filePath = IG_PROFILE_FOLDER + IG_PENDING_FOLDER + steamID + ".json";
        
        Print("[DEBUG] Checking pending items for: " + steamID);
        
        if (!FileExist(filePath)) 
        {
            Print("[DEBUG] No file found: " + filePath);
            return false;
        }
        
        ItemQueue itemQueue = new ItemQueue();
        JsonFileLoader<ItemQueue>.JsonLoadFile(filePath, itemQueue);
        
        if (!itemQueue || !itemQueue.items_to_give) 
        {
            Print("[DEBUG] No itemQueue or items_to_give");
            return false;
        }
        
        Print("[DEBUG] Items count: " + itemQueue.items_to_give.Count().ToString());
        
        // Check if there are any items to give
        for (int i = 0; i < itemQueue.items_to_give.Count(); i++)
        {
            ItemInfo item = itemQueue.items_to_give.Get(i);
            if (item && item.classname != "" && item.quantity > 0)
            {
                Print("[DEBUG] Found pending item: " + item.classname + " (status: " + item.status + ")");
                return true;
            }
        }
        
        Print("[DEBUG] No valid pending items found");
        return false;
    }

    // ===== ENHANCED TERRITORY VALIDATION SYSTEM WITH DISTANCE CALCULATION =====
    static bool IsPlayerInValidTerritory(PlayerBase player, out float distanceToPlotpole)
    {
        distanceToPlotpole = -1;
        
        if (!player || !player.GetIdentity()) 
        {
            Print("[NightroItemGiver] Player or identity is null");
            return false;
        }

        string steamID = player.GetIdentity().GetPlainId();
        Print("[NightroItemGiver] Checking territory for player: " + steamID);

        // Check if territory validation is enabled
        if (!m_NotificationSettings.require_territory_check)
        {
            Print("[NightroItemGiver] Territory check disabled, allowing item delivery");
            return true;
        }

        // ===== ONLY SEND NOTIFICATIONS IF PLAYER HAS PENDING ITEMS =====
        bool hasPendingItems = HasPendingItems(player);
        Print("[DEBUG] Player " + steamID + " has pending items: " + hasPendingItems.ToString());
        
        if (!hasPendingItems)
        {
            Print("[NightroItemGiver] Player " + steamID + " has no pending items, skipping territory notifications");
            return false; // Still return false for territory validation, but don't send notifications
        }

        // Get player's LBmaster group
        LBGroup playerGroup = GetPlayerGroup(player);
        if (!playerGroup)
        {
            Print("[NightroItemGiver] Player " + steamID + " is not in any group");
            
            // Calculate distance to nearest plotpole
            distanceToPlotpole = GetDistanceToNearestPlotpole(player.GetPosition());
            
            string noGroupMsg = m_NotificationSettings.no_group_message;
            noGroupMsg.Replace("{DISTANCE}", ((int)distanceToPlotpole).ToString());
            SendPeriodicNotification(player, noGroupMsg, "no_group", m_NotificationSettings.no_group_notification_interval);
            return false;
        }

        string groupTag = playerGroup.shortname;
        Print("[NightroItemGiver] Player " + steamID + " is in group: " + groupTag);

        // Check if group has any plotpoles
        if (!GroupHasPlotpole(playerGroup))
        {
            Print("[NightroItemGiver] Group " + groupTag + " has no plotpoles");
            SendPeriodicNotification(player, m_NotificationSettings.no_plotpole_message, "no_plotpole", m_NotificationSettings.no_plotpole_notification_interval);
            return false;
        }

        // Check if player is in their group's territory using LBmaster's plotpole system
        bool isInTerritory = IsPlayerInGroupTerritory(player, playerGroup, distanceToPlotpole);
        if (!isInTerritory)
        {
            Print("[NightroItemGiver] Player " + steamID + " is outside their group's territory. Distance: " + distanceToPlotpole + "m");
            
            string territoryMsg = m_NotificationSettings.outside_territory_message;
            territoryMsg.Replace("{DISTANCE}", ((int)distanceToPlotpole).ToString());
            SendPeriodicNotification(player, territoryMsg, "outside_territory", m_NotificationSettings.outside_territory_notification_interval);
            return false;
        }

        Print("[NightroItemGiver] Player " + steamID + " is in valid territory");
        return true;
    }

    // Get distance to nearest plotpole
    static float GetDistanceToNearestPlotpole(vector playerPos)
    {
        float nearestDistance = 999999.0;
        
        #ifdef LBmaster_GroupDLCPlotpole
        foreach (TerritoryFlag flag : TerritoryFlag.all_Flags)
        {
            if (!flag) continue;
            
            float distance = vector.Distance(playerPos, flag.GetPosition());
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
            }
        }
        #endif
        
        return nearestDistance;
    }

    // Check if group has any plotpoles
    static bool GroupHasPlotpole(LBGroup playerGroup)
    {
        if (!playerGroup) return false;
        
        #ifdef LBmaster_GroupDLCPlotpole
        string tagLower = playerGroup.shortname + "";
        tagLower.ToLower();
        int groupTagHash = tagLower.Hash();
        
        foreach (TerritoryFlag flag : TerritoryFlag.all_Flags)
        {
            if (!flag) continue;
            
            if (flag.ownerGroupTagHash == groupTagHash)
            {
                return true;
            }
        }
        #endif
        
        return false;
    }

    // Get player's group using LBmaster system
    static LBGroup GetPlayerGroup(PlayerBase player)
    {
        if (!player)
            return null;
            
        // Use LBmaster's built-in method to get player's group
        return player.GetLBGroup();
    }

    // Check if player is in their group's territory using LBmaster plotpole system
    static bool IsPlayerInGroupTerritory(PlayerBase player, LBGroup playerGroup, out float distanceToNearestFriendlyFlag)
    {
        distanceToNearestFriendlyFlag = 999999.0;
        
        if (!player || !playerGroup) 
            return false;

        vector playerPos = player.GetPosition();
        string groupTag = playerGroup.shortname;
        
        Print("[NightroItemGiver] Checking territory for group: " + groupTag);
        Print("[NightroItemGiver] Player position: " + playerPos);

        #ifdef LBmaster_GroupDLCPlotpole
        // Get group tag hash as used by LBmaster
        string groupTagLower = groupTag;
        groupTagLower.ToLower();
        int groupTagHash = groupTagLower.Hash();
        
        // Find the nearest friendly flag using LBmaster's system
        TerritoryFlag nearestFriendlyFlag = TerritoryFlag.FindNearestFlag(playerPos, true, true, groupTagHash);
        
        if (nearestFriendlyFlag)
        {
            distanceToNearestFriendlyFlag = vector.Distance(playerPos, nearestFriendlyFlag.GetPosition());
            
            if (nearestFriendlyFlag.IsInRadius(playerPos))
            {
                Print("[NightroItemGiver] Player is within friendly territory flag radius. Distance: " + distanceToNearestFriendlyFlag + "m");
                return true;
            }
        }
        
        Print("[NightroItemGiver] No friendly territory flag found or player is outside radius. Distance to nearest: " + distanceToNearestFriendlyFlag + "m");
        return false;
        #else
        Print("[NightroItemGiver] LBmaster Plotpole DLC not enabled, allowing item delivery");
        return true;
        #endif
    }

    // Enhanced notification system with periodic reminders
    static void SendPeriodicNotification(PlayerBase player, string notificationMessage, string notificationType, int intervalSeconds)
    {
        if (!player || !player.GetIdentity()) return;
        if (!m_NotificationSettings.enable_periodic_notifications) return;

        string steamID = player.GetIdentity().GetPlainId();
        float currentTime = GetGame().GetTime();
        
        if (!m_PlayerStates.Contains(steamID))
        {
            m_PlayerStates.Set(steamID, new PlayerStateInfo());
        }
        
        PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
        
        // Check if enough time has passed since last notification of this type
        bool shouldNotify = false;
        if (playerState.last_notification_type != notificationType)
        {
            shouldNotify = true; // First time notification of this type
        }
        else if ((currentTime - playerState.last_notification_time) >= (intervalSeconds * 1000))
        {
            shouldNotify = true; // Interval has passed
        }
        
        if (shouldNotify)
        {
            // Send chat message
            GetGame().ChatMP(player, notificationMessage, "ColorRed");
            Print("[NightroItemGiver] Sent periodic notification to player " + steamID + ": " + notificationType + " - " + notificationMessage);
            
            // Update notification tracking
            playerState.last_notification_time = currentTime;
            playerState.last_notification_type = notificationType;
        }
    }

    static void SendSuccessNotification(PlayerBase player, string itemName, int quantity)
    {
        if (!player || !player.GetIdentity()) return;

        string successMsg = m_NotificationSettings.item_delivered_message;
        successMsg.Replace("{ITEM_NAME}", itemName);
        successMsg.Replace("{QUANTITY}", quantity.ToString());
        
        GetGame().ChatMP(player, successMsg, "ColorGreen");
        Print("[NightroItemGiver] Sent success notification to player: " + successMsg);
        
        // ===== CLEAR NOTIFICATION STATE WHEN ITEMS ARE DELIVERED =====
        string steamID = player.GetIdentity().GetPlainId();
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
            Print("[NightroItemGiver] Cleared notification state for delivered items");
        }
    }

    static void SendInventoryFullNotification(PlayerBase player, string itemName, int quantity)
    {
        if (!player || !player.GetIdentity()) return;

        string steamID = player.GetIdentity().GetPlainId();
        string inventoryMsg = m_NotificationSettings.inventory_full_message;
        inventoryMsg.Replace("{ITEM_NAME}", itemName);
        inventoryMsg.Replace("{QUANTITY}", quantity.ToString());
        
        SendPeriodicNotification(player, inventoryMsg, "inventory_full", m_NotificationSettings.inventory_full_notification_interval);
    }

    static void SendMaxRetryReachedNotification(PlayerBase player, string itemName, int quantity)
    {
        if (!player || !player.GetIdentity()) return;

        string retryMsg = m_NotificationSettings.max_retry_reached_message;
        retryMsg.Replace("{ITEM_NAME}", itemName);
        retryMsg.Replace("{QUANTITY}", quantity.ToString());
        
        GetGame().ChatMP(player, retryMsg, "ColorYellow");
        Print("[NightroItemGiver] Sent max retry reached notification: " + retryMsg);
        
        // ===== CLEAR NOTIFICATION STATE WHEN ITEMS ARE REMOVED =====
        string steamID = player.GetIdentity().GetPlainId();
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
            Print("[NightroItemGiver] Cleared notification state for removed items");
        }
    }

    static bool IsPlayerReadyForItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return false;
        
        string steamID = player.GetIdentity().GetPlainId();
        
        Print("[DEBUG] Checking if player " + steamID + " is ready for items");
        
        // Basic state checks
        if (player.IsUnconscious() || player.IsRestrained() || player.IsAlive() == false)
        {
            Print("[NightroItemGiver] Player " + steamID + " is not in valid state (unconscious/dead/restrained)");
            return false;
        }
        
        if (player.IsInVehicle() && player.GetParent())
        {
            Print("[NightroItemGiver] Player " + steamID + " is in vehicle/special state, waiting...");
            return false;
        }

        // Territory validation check with distance calculation
        float distanceToPlotpole;
        if (!IsPlayerInValidTerritory(player, distanceToPlotpole))
        {
            Print("[NightroItemGiver] Player " + steamID + " failed territory validation. Distance to plotpole: " + distanceToPlotpole + "m");
            return false;
        }
        
        // Time-based checks ONLY IF PLAYER HAS PENDING ITEMS
        bool hasPendingItems = HasPendingItems(player);
        Print("[DEBUG] Player " + steamID + " has pending items for time check: " + hasPendingItems.ToString());
        
        if (!hasPendingItems)
        {
            Print("[NightroItemGiver] Player " + steamID + " has no pending items");
            return false;
        }
        
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            float currentTime = GetGame().GetTime();
            
            float timeSinceConnect = currentTime - playerState.last_connect_time;
            if (timeSinceConnect < m_NotificationSettings.initial_delay_seconds * 1000)
            {
                Print("[NightroItemGiver] Player " + steamID + " connected " + (timeSinceConnect/1000).ToString() + "s ago, waiting for initial delay (" + m_NotificationSettings.initial_delay_seconds + "s)");
                
                string waitingMsg = m_NotificationSettings.waiting_conditions_message;
                SendPeriodicNotification(player, waitingMsg, "waiting_conditions", m_NotificationSettings.waiting_conditions_notification_interval);
                return false;
            }
            
            if (playerState.last_respawn_time > 0)
            {
                float timeSinceRespawn = currentTime - playerState.last_respawn_time;
                if (timeSinceRespawn < m_NotificationSettings.respawn_delay_seconds * 1000)
                {
                    Print("[NightroItemGiver] Player " + steamID + " respawned " + (timeSinceRespawn/1000).ToString() + "s ago, waiting for respawn delay (" + m_NotificationSettings.respawn_delay_seconds + "s)");
                    
                    string respawnWaitingMsg = m_NotificationSettings.waiting_conditions_message;
                    SendPeriodicNotification(player, respawnWaitingMsg, "waiting_conditions", m_NotificationSettings.waiting_conditions_notification_interval);
                    return false;
                }
            }
            
            playerState.is_ready_for_items = true;
        }
        
        Print("[NightroItemGiver] Player " + steamID + " is ready for items");
        return true;
    }

    static void UpdatePlayerState(PlayerBase player, string action)
    {
        if (!player || !player.GetIdentity()) return;
        
        string steamID = player.GetIdentity().GetPlainId();
        float currentTime = GetGame().GetTime();
        
        if (!m_PlayerStates.Contains(steamID))
        {
            m_PlayerStates.Set(steamID, new PlayerStateInfo());
        }
        
        PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
        playerState.steam_id = steamID;
        
        if (action == "connect")
        {
            playerState.last_connect_time = currentTime;
            playerState.connect_count++;
            playerState.is_ready_for_items = false;
            playerState.last_notification_time = 0; // Reset notification timer on connect
            playerState.last_notification_type = "";
            
            if (!player.IsAlive())
            {
                playerState.was_dead_on_connect = true;
                Print("[NightroItemGiver] Player " + steamID + " connected while dead, will wait for respawn");
            }
            else
            {
                playerState.was_dead_on_connect = false;
                Print("[NightroItemGiver] Player " + steamID + " connected alive, starting initial delay");
            }
        }
        else if (action == "respawn")
        {
            playerState.last_respawn_time = currentTime;
            playerState.is_ready_for_items = false;
            playerState.was_dead_on_connect = false;
            playerState.last_notification_time = 0; // Reset notification timer on respawn
            playerState.last_notification_type = "";
            Print("[NightroItemGiver] Player " + steamID + " respawned, starting respawn delay");
        }
        
        Print("[NightroItemGiver] Updated state for " + steamID + " - Action: " + action + ", Ready: " + playerState.is_ready_for_items.ToString());
    }
    
    static void CheckAllOnlinePlayers()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        Print("===== ENHANCED AUTO-CHECK WITH ADVANCED TERRITORY VALIDATION V2.5 =====");
        Print("[NightroItemGiver] Checking " + players.Count().ToString() + " online players for pending items...");
        
        for (int i = 0; i < players.Count(); i++)
        {
            Man p = players.Get(i);
            PlayerBase player = PlayerBase.Cast(p);
            if (player && player.GetIdentity())
            {
                Print("[DEBUG] Processing player: " + player.GetIdentity().GetPlainId());
                // ===== ALWAYS TRY TO CHECK AND GIVE ITEMS =====
                CheckAndGiveItems(player);
            }
        }
    }
    
    static string GetCurrentTimestamp()
    {
        float timestamp = GetGame().GetTime();
        return timestamp.ToString();
    }
    
    static void UpdateItemStatus(ref ItemInfo itemInfo, string status, string errorMessage = "")
    {
        itemInfo.status = status;
        itemInfo.processed_at = GetCurrentTimestamp();
        itemInfo.error_message = errorMessage;
        
        Print("[NightroItemGiver] Updated item status: " + itemInfo.classname + " -> " + status);
        if (errorMessage != "")
        {
            Print("[NightroItemGiver] Error message: " + errorMessage);
        }
    }

    static void UpdateAttachmentStatus(ref AttachmentInfo attachment, string status)
    {
        attachment.status = status;
        Print("[NightroItemGiver] Updated attachment status: " + attachment.classname + " -> " + status);
    }

    static void UpdateItemRetry(ref ItemInfo itemInfo, string errorMessage = "")
    {
        itemInfo.retry_count++;
        itemInfo.last_retry_at = GetGame().GetTime().ToString();
        itemInfo.error_message = errorMessage;
        
        Print("[NightroItemGiver] Updated retry for " + itemInfo.classname + " - Attempt: " + itemInfo.retry_count.ToString() + "/" + m_NotificationSettings.max_retry_attempts.ToString());
        Print("[NightroItemGiver] Next retry available in: " + m_NotificationSettings.retry_interval_seconds.ToString() + " seconds");
    }

    static bool ShouldRetryItem(ref ItemInfo itemInfo)
    {
        // เช็คจำนวนครั้ง
        if (itemInfo.retry_count >= m_NotificationSettings.max_retry_attempts)
        {
            Print("[NightroItemGiver] Max retry attempts reached for " + itemInfo.classname);
            return false;
        }
        
        // ถ้ายังไม่เคยลอง ให้ลองได้เลย
        if (itemInfo.last_retry_at == "" || itemInfo.retry_count == 0)
        {
            Print("[NightroItemGiver] First retry attempt for " + itemInfo.classname);
            return true;
        }
        
        // เช็คเวลาที่ผ่านไป
        float currentTime = GetGame().GetTime();
        float lastRetryTime = itemInfo.last_retry_at.ToFloat();
        float timeSinceLastRetry = currentTime - lastRetryTime;
        float retryIntervalMs = m_NotificationSettings.retry_interval_seconds * 1000;
        
        if (timeSinceLastRetry >= retryIntervalMs)
        {
            Print("[NightroItemGiver] Retry interval passed for " + itemInfo.classname + " (" + (timeSinceLastRetry/1000).ToString() + "s >= " + m_NotificationSettings.retry_interval_seconds.ToString() + "s)");
            return true;
        }
        
        Print("[NightroItemGiver] Still waiting for retry interval for " + itemInfo.classname + " (" + (timeSinceLastRetry/1000).ToString() + "s / " + m_NotificationSettings.retry_interval_seconds.ToString() + "s)");
        return false;
    }
    
    static void MoveItemToProcessed(ref ItemQueue itemQueue, ref ItemInfo itemInfo)
    {
        ProcessedItemInfo processedItem = new ProcessedItemInfo();
        processedItem.classname = itemInfo.classname;
        processedItem.quantity = itemInfo.quantity;
        processedItem.status = itemInfo.status;
        processedItem.processed_at = itemInfo.processed_at;
        processedItem.error_message = itemInfo.error_message;
        
        if (itemInfo.attachments && itemInfo.attachments.Count() > 0)
        {
            processedItem.attachments = new array<ref AttachmentInfo>;
            for (int i = 0; i < itemInfo.attachments.Count(); i++)
            {
                AttachmentInfo originalAttachment = itemInfo.attachments.Get(i);
                AttachmentInfo copyAttachment = new AttachmentInfo();
                copyAttachment.classname = originalAttachment.classname;
                copyAttachment.quantity = originalAttachment.quantity;
                copyAttachment.status = originalAttachment.status;
                
                if (originalAttachment.attachments && originalAttachment.attachments.Count() > 0)
                {
                    copyAttachment.attachments = new array<ref AttachmentInfo>;
                    for (int j = 0; j < originalAttachment.attachments.Count(); j++)
                    {
                        AttachmentInfo nestedAttachment = originalAttachment.attachments.Get(j);
                        AttachmentInfo copyNestedAttachment = new AttachmentInfo();
                        copyNestedAttachment.classname = nestedAttachment.classname;
                        copyNestedAttachment.quantity = nestedAttachment.quantity;
                        copyNestedAttachment.status = nestedAttachment.status;
                        copyAttachment.attachments.Insert(copyNestedAttachment);
                    }
                }
                
                processedItem.attachments.Insert(copyAttachment);
            }
        }
        
        itemQueue.processed_items.Insert(processedItem);
        
        Print("[NightroItemGiver] Moved item to processed: " + itemInfo.classname + " (" + itemInfo.status + ")");
    }
    
    static bool IsValidItemClass(string itemName)
    {
        if (!itemName || itemName == "") return false;

        EntityAI testItem = EntityAI.Cast(GetGame().CreateObjectEx(itemName, "0 0 0", ECE_LOCAL | ECE_NOLIFETIME));
        if (!testItem)
        {
            return false;
        }

        GetGame().ObjectDelete(testItem);
        return true;
    }
    
    static bool CanFitInInventory(PlayerBase player, string itemClassname, int quantity)
    {
        if (!player || !player.GetInventory()) return false;
        
        for (int i = 0; i < quantity; i++)
        {
            EntityAI testItem = EntityAI.Cast(GetGame().CreateObjectEx(itemClassname, "0 0 0", ECE_LOCAL | ECE_NOLIFETIME));
            if (!testItem)
            {
                Print("[NightroItemGiver] Cannot create test item: " + itemClassname);
                return false;
            }

            InventoryLocation invLocation = new InventoryLocation();
            bool canFit = player.GetInventory().FindFreeLocationFor(testItem, FindInventoryLocationType.ANY, invLocation);
            GetGame().ObjectDelete(testItem);
            
            if (!canFit)
            {
                Print("[NightroItemGiver] Cannot fit item " + (i+1) + "/" + quantity + " of " + itemClassname);
                return false;
            }
        }

        Print("[NightroItemGiver] Can fit all " + quantity + "x " + itemClassname);
        return true;
    }
    
    static bool IsMagazine(string itemName)
    {
        if (itemName == "") return false;
        return GetGame().ConfigIsExisting("CfgMagazines " + itemName);
    }
    
    static bool CanAcceptMagazine(Weapon_Base weapon, string magazineClass)
    {
        if (!weapon || magazineClass == "") return false;

        array<string> compatibleMagazines = {};
        GetGame().ConfigGetTextArray("CfgWeapons " + weapon.GetType() + " magazines", compatibleMagazines);

        return compatibleMagazines.Find(magazineClass) > -1;
    }
    
    static void ValidateWeaponState(Weapon_Base weapon)
    {
        if (!weapon) return;
        
        Print("[NightroItemGiver] Validating weapon state for: " + weapon.GetType());
        
        Magazine currentMag = weapon.GetMagazine(0);
        if (currentMag)
        {
            string magType = currentMag.GetType();
            Print("[NightroItemGiver] Current magazine: " + magType);
            
            array<string> validMags = {};
            GetGame().ConfigGetTextArray("CfgWeapons " + weapon.GetType() + " magazines", validMags);
            
            if (validMags.Find(magType) == -1)
            {
                Print("[NightroItemGiver] Invalid magazine detected, removing: " + magType);
                weapon.ServerDropEntity(currentMag);
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().ObjectDelete, 1000, false, currentMag);
            }
            else
            {
                currentMag.ServerSetAmmoCount(currentMag.GetAmmoMax());
                Print("[NightroItemGiver] Magazine validated and loaded with ammo: " + magType);
            }
        }
        
        weapon.ValidateAndRepair();
        weapon.SetSynchDirty();
        weapon.Synchronize();
    }
    
    static void AttachItemsToWeapon(EntityAI parentItem, array<ref AttachmentInfo> attachments, PlayerBase player)
    {
        if (!parentItem || !attachments || !player) return;
        
        Print("===== INSTALLING ATTACHMENTS (V2.5 COMPLETE) =====");
        Print("[NightroItemGiver] Processing " + attachments.Count().ToString() + " attachments for " + parentItem.GetType());
        
        for (int i = 0; i < attachments.Count(); i++)
        {
            AttachmentInfo attachment = attachments.Get(i);
            if (!attachment || attachment.classname == "") continue;
            
            Print("[NightroItemGiver] Processing attachment " + (i+1) + "/" + attachments.Count() + ": " + attachment.classname);
            
            if (!IsValidItemClass(attachment.classname))
            {
                Print("[NightroItemGiver] Invalid attachment: " + attachment.classname);
                UpdateAttachmentStatus(attachment, "failed");
                continue;
            }
            
            bool attachmentSuccess = false;
            
            for (int j = 0; j < attachment.quantity; j++)
            {
                if (Weapon_Base.Cast(parentItem) && IsMagazine(attachment.classname))
                {
                    Weapon_Base weapon = Weapon_Base.Cast(parentItem);
                    
                    if (CanAcceptMagazine(weapon, attachment.classname))
                    {
                        Magazine existingMag = weapon.GetMagazine(0);
                        if (existingMag)
                        {
                            Print("[NightroItemGiver] Weapon already has magazine: " + existingMag.GetType() + ", replacing with: " + attachment.classname);
                            weapon.ServerDropEntity(existingMag);
                            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().ObjectDelete, 500, false, existingMag);
                        }
                        
                        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(InstallMagazineDelayed, 800, false, weapon, attachment.classname, attachment);
                        attachmentSuccess = true;
                    }
                    else
                    {
                        Print("[NightroItemGiver] Magazine " + attachment.classname + " not compatible with " + weapon.GetType());
                        EntityAI playerMagItem = player.GetInventory().CreateInInventory(attachment.classname);
                        if (playerMagItem)
                        {
                            Magazine mag = Magazine.Cast(playerMagItem);
                            if (mag) mag.ServerSetAmmoCount(mag.GetAmmoMax());
                            Print("[NightroItemGiver] Added incompatible magazine to player inventory: " + attachment.classname);
                            UpdateAttachmentStatus(attachment, "delivered");
                            attachmentSuccess = true;
                        }
                        else
                        {
                            UpdateAttachmentStatus(attachment, "failed");
                        }
                    }
                }
                else
                {
                    EntityAI attachmentEntity = parentItem.GetInventory().CreateAttachment(attachment.classname);
                    if (attachmentEntity)
                    {
                        Print("[NightroItemGiver] Successfully attached: " + attachment.classname);
                        UpdateAttachmentStatus(attachment, "delivered");
                        attachmentSuccess = true;
                        
                        if (attachment.attachments && attachment.attachments.Count() > 0)
                        {
                            Print("[NightroItemGiver] Processing " + attachment.attachments.Count() + " nested attachments for: " + attachment.classname);
                            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AttachItemsToWeapon, 500, false, attachmentEntity, attachment.attachments, player);
                        }
                    }
                    else
                    {
                        Print("[NightroItemGiver] Failed to attach: " + attachment.classname + ", trying player inventory");
                        EntityAI playerAttachmentItem = player.GetInventory().CreateInInventory(attachment.classname);
                        if (playerAttachmentItem)
                        {
                            Print("[NightroItemGiver] Added to player inventory: " + attachment.classname);
                            UpdateAttachmentStatus(attachment, "delivered");
                            attachmentSuccess = true;
                            
                            if (attachment.attachments && attachment.attachments.Count() > 0)
                            {
                                Print("[NightroItemGiver] Processing " + attachment.attachments.Count() + " nested attachments for inventory item: " + attachment.classname);
                                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AttachItemsToWeapon, 500, false, playerAttachmentItem, attachment.attachments, player);
                            }
                        }
                        else
                        {
                            UpdateAttachmentStatus(attachment, "failed");
                        }
                    }
                }
            }
            
            if (!attachmentSuccess)
            {
                Print("[NightroItemGiver] Failed to install attachment: " + attachment.classname);
            }
        }
        
        Print("===== ATTACHMENT INSTALLATION COMPLETE =====");
    }
    
    static void InstallMagazineDelayed(Weapon_Base weapon, string magazineClass, ref AttachmentInfo attachment)
    {
        if (!weapon || magazineClass == "" || !attachment) return;
        
        Print("[NightroItemGiver] Installing magazine: " + magazineClass + " to " + weapon.GetType());
        
        Magazine existingMag = weapon.GetMagazine(0);
        if (existingMag)
        {
            Print("[NightroItemGiver] Magazine already exists, skipping installation");
            return;
        }
        
        EntityAI newMag = weapon.GetInventory().CreateAttachment(magazineClass);
        if (newMag)
        {
            Magazine magazine = Magazine.Cast(newMag);
            if (magazine)
            {
                magazine.ServerSetAmmoCount(magazine.GetAmmoMax());
                Print("[NightroItemGiver] Successfully installed and loaded magazine: " + magazineClass);
                UpdateAttachmentStatus(attachment, "delivered");
                
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ValidateWeaponState, 500, false, weapon);
            }
        }
        else
        {
            Print("[NightroItemGiver] Failed to create magazine: " + magazineClass);
            UpdateAttachmentStatus(attachment, "failed");
        }
    }
    
    static void CheckAndGiveItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return;

        string steamID = player.GetIdentity().GetPlainId();
        string filePath = IG_PROFILE_FOLDER + IG_PENDING_FOLDER + steamID + ".json";

        Print("===== ENHANCED VERSION 2.5 WITH ADVANCED TERRITORY VALIDATION AND NOTIFICATIONS (COMPLETE) =====");
        Print("[NightroItemGiver] CHECKING ITEMS FOR: " + steamID);
        Print("[NightroItemGiver] File path: " + filePath);
        Print("[NightroItemGiver] File exists: " + FileExist(filePath).ToString());

        if (!FileExist(filePath))
        {
            Print("[NightroItemGiver] No pending items file for player: " + steamID);
            return;
        }

        Print("[NightroItemGiver] Found pending items for: " + steamID);
        
        ItemQueue itemQueue = new ItemQueue();
        JsonFileLoader<ItemQueue>.JsonLoadFile(filePath, itemQueue);
        
        if (!itemQueue)
        {
            Print("[NightroItemGiver] Failed to load JSON file - file may be corrupted");
            ClearJsonFile(filePath, player);
            return;
        }
        
        Print("===== JSON LOADED SUCCESSFULLY =====");
        Print("[NightroItemGiver] Player name: " + itemQueue.player_name);
        Print("[NightroItemGiver] Steam ID: " + itemQueue.steam_id);
        
        if (!itemQueue.items_to_give)
        {
            Print("[NightroItemGiver] ERROR: items_to_give is NULL!");
            itemQueue.items_to_give = new array<ref ItemInfo>;
            return;
        }
        
        if (!itemQueue.processed_items)
        {
            itemQueue.processed_items = new array<ref ProcessedItemInfo>;
            Print("[NightroItemGiver] Initialized processed_items array");
        }

        if (!itemQueue.player_state)
        {
            itemQueue.player_state = new PlayerStateInfo();
            itemQueue.player_state.steam_id = steamID;
            Print("[NightroItemGiver] Initialized player state tracking");
        }
        
        Print("===== ITEMS TO GIVE COUNT: " + itemQueue.items_to_give.Count().ToString() + " =====");
        Print("===== PROCESSED ITEMS COUNT: " + itemQueue.processed_items.Count().ToString() + " =====");
        
        if (itemQueue.items_to_give.Count() > 0)
        {
            Print("[DEBUG] Player has " + itemQueue.items_to_give.Count().ToString() + " items to process");
            
            // ===== ALWAYS PROCESS ITEMS, REGARDLESS OF PLAYER READINESS =====
            // This allows retry system to work even when player is not ready
            array<ref ItemInfo> itemsToProcess = new array<ref ItemInfo>;
            array<ref ItemInfo> remainingItems = new array<ref ItemInfo>;
            
            // ===== CHECK IF PLAYER IS READY FOR ACTUAL DELIVERY =====
            bool playerReadyForDelivery = IsPlayerReadyForItems(player);
            Print("[DEBUG] Player ready for delivery: " + playerReadyForDelivery.ToString());
            
            for (int itemIndex = 0; itemIndex < itemQueue.items_to_give.Count(); itemIndex++)
            {
                ItemInfo currentItem = itemQueue.items_to_give.Get(itemIndex);
                
                if (currentItem && currentItem.classname != "" && currentItem.quantity > 0)
                {
                    Print("[DEBUG] Processing item: " + currentItem.classname + " (status: " + currentItem.status + ", retry: " + currentItem.retry_count.ToString() + ")");
                    
                    if (currentItem.status == "pending" || currentItem.status == "")
                    {
                        if (playerReadyForDelivery)
                        {
                            itemsToProcess.Insert(currentItem);
                            Print("[NightroItemGiver] Added to process queue: " + currentItem.classname + " (status: " + currentItem.status + ")");
                        }
                        else
                        {
                            // Check if this retry attempt would exceed max attempts
                            if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                            {
                                Print("[NightroItemGiver] Max retry attempts reached for " + currentItem.classname + " - moving to processed");
                                UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - player not ready for delivery");
                                MoveItemToProcessed(itemQueue, currentItem);
                                SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                            }
                            else
                            {
                                // Player not ready, convert to territory_failed for retry tracking
                                UpdateItemRetry(currentItem, "Player not ready for delivery");
                                currentItem.status = "territory_failed";
                                remainingItems.Insert(currentItem);
                                Print("[NightroItemGiver] Player not ready - will retry later (" + currentItem.retry_count.ToString() + "/" + m_NotificationSettings.max_retry_attempts.ToString() + ")");
                            }
                        }
                    }
                    else if ((currentItem.status == "inventory_full" || currentItem.status == "territory_failed") && ShouldRetryItem(currentItem))
                    {
                        if (playerReadyForDelivery)
                        {
                            itemsToProcess.Insert(currentItem);
                            Print("[NightroItemGiver] Added to retry queue: " + currentItem.classname + " (retry: " + currentItem.retry_count.ToString() + ", status: " + currentItem.status + ")");
                        }
                        else
                        {
                            // Check if this retry attempt would exceed max attempts
                            if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                            {
                                Print("[NightroItemGiver] Max retry attempts reached for " + currentItem.classname + " - moving to processed");
                                UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - player not ready for delivery");
                                MoveItemToProcessed(itemQueue, currentItem);
                                SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                            }
                            else
                            {
                                UpdateItemRetry(currentItem, "Player still not ready for delivery");
                                remainingItems.Insert(currentItem);
                                Print("[NightroItemGiver] Player still not ready - will retry later (" + currentItem.retry_count.ToString() + "/" + m_NotificationSettings.max_retry_attempts.ToString() + ")");
                            }
                        }
                    }
                    else if (currentItem.status == "inventory_full" || currentItem.status == "territory_failed")
                    {
                        // Check if max retry reached
                        if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                        {
                            Print("[NightroItemGiver] Max retry attempts reached for " + currentItem.classname + " - moving to processed");
                            UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - " + currentItem.error_message);
                            MoveItemToProcessed(itemQueue, currentItem);
                            SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                        }
                        else
                        {
                            Print("[NightroItemGiver] Retry not ready yet: " + currentItem.classname + " (retry: " + currentItem.retry_count.ToString() + ", status: " + currentItem.status + ")");
                            remainingItems.Insert(currentItem);
                        }
                    }
                    else
                    {
                        Print("[NightroItemGiver] Skipping non-pending item: " + currentItem.classname + " (status: " + currentItem.status + ")");
                        if (currentItem.status == "delivered" || currentItem.status == "failed")
                        {
                            MoveItemToProcessed(itemQueue, currentItem);
                        }
                        else
                        {
                            remainingItems.Insert(currentItem);
                        }
                    }
                }
            }
            
            // Process pending items
            for (int processIndex = 0; processIndex < itemsToProcess.Count(); processIndex++)
            {
                ItemInfo processItem = itemsToProcess.Get(processIndex);
                
                Print("===== PROCESSING ITEM " + processIndex.ToString() + " =====");
                Print("[NightroItemGiver] Processing: " + processItem.classname + " (qty: " + processItem.quantity.ToString() + ")");
                
                if (!IsValidItemClass(processItem.classname))
                {
                    Print("[NightroItemGiver] Invalid classname: " + processItem.classname + " - MARKING AS FAILED");
                    UpdateItemStatus(processItem, "failed", "Invalid classname: " + processItem.classname);
                    MoveItemToProcessed(itemQueue, processItem);
                    continue;
                }

                // Final territory check before giving items
                float distanceToPlotpole;
                if (!IsPlayerInValidTerritory(player, distanceToPlotpole))
                {
                    Print("[NightroItemGiver] Player failed final territory check, updating item status");
                    UpdateItemRetry(processItem, "Player not in valid territory");
                    
                    if (processItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                    {
                        UpdateItemStatus(processItem, "failed", "Max retry attempts reached - territory validation failed");
                        MoveItemToProcessed(itemQueue, processItem);
                        SendMaxRetryReachedNotification(player, processItem.classname, processItem.quantity);
                        Print("[NightroItemGiver] Max retry attempts reached for territory validation: " + processItem.classname);
                    }
                    else
                    {
                        processItem.status = "territory_failed";
                        remainingItems.Insert(processItem);
                        Print("[NightroItemGiver] Territory check failed - will retry later (" + processItem.retry_count.ToString() + "/" + m_NotificationSettings.max_retry_attempts.ToString() + ")");
                    }
                    continue;
                }
                
                if (CanFitInInventory(player, processItem.classname, processItem.quantity))
                {
                    int itemsGiven = 0;
                    for (int i = 0; i < processItem.quantity; i++)
                    {
                        EntityAI item = player.GetInventory().CreateInInventory(processItem.classname);
                        if (item)
                        {
                            itemsGiven++;
                            Print("[NightroItemGiver] Successfully created item: " + processItem.classname);
                            
                            if (processItem.attachments && processItem.attachments.Count() > 0)
                            {
                                Print("===== STARTING ATTACHMENT INSTALLATION =====");
                                Print("[NightroItemGiver] Will install " + processItem.attachments.Count() + " attachments");
                                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AttachItemsToWeapon, 300, false, item, processItem.attachments, player);
                            }
                        }
                        else
                        {
                            Print("[NightroItemGiver] Failed to create item: " + processItem.classname);
                            break;
                        }
                    }
                    
                    if (itemsGiven > 0)
                    {
                        SendSuccessNotification(player, processItem.classname, itemsGiven);
                        Print("[NightroItemGiver] Gave " + itemsGiven.ToString() + "x " + processItem.classname + " to " + player.GetIdentity().GetName());
                        
                        UpdateItemStatus(processItem, "delivered");
                        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DelayedMoveToProcessed, 5000, false, itemQueue, processItem);
                        
                        if (itemsGiven < processItem.quantity)
                        {
                            ItemInfo remainingItem = new ItemInfo();
                            remainingItem.classname = processItem.classname;
                            remainingItem.quantity = processItem.quantity - itemsGiven;
                            remainingItem.attachments = processItem.attachments;
                            remainingItem.status = "pending";
                            remainingItems.Insert(remainingItem);
                        }
                    }
                    else
                    {
                        UpdateItemStatus(processItem, "failed", "Failed to create item in inventory");
                        MoveItemToProcessed(itemQueue, processItem);
                    }
                }
                else
                {
                    UpdateItemRetry(processItem, "Inventory full");
                    
                    if (processItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                    {
                        UpdateItemStatus(processItem, "failed", "Max retry attempts reached - inventory full");
                        MoveItemToProcessed(itemQueue, processItem);
                        SendMaxRetryReachedNotification(player, processItem.classname, processItem.quantity);
                        Print("[NightroItemGiver] Max retry attempts reached for " + processItem.classname);
                    }
                    else
                    {
                        processItem.status = "inventory_full";
                        remainingItems.Insert(processItem);
                        SendInventoryFullNotification(player, processItem.classname, processItem.quantity);
                        Print("[NightroItemGiver] Inventory full - will retry later (" + processItem.retry_count.ToString() + "/" + m_NotificationSettings.max_retry_attempts.ToString() + ")");
                    }
                }
            }
            
            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DelayedUpdateJsonFile, 6000, false, filePath, remainingItems, itemQueue, player);
        }
        else
        {
            Print("[NightroItemGiver] No pending items to process");
        }
    }

    static void DelayedMoveToProcessed(ref ItemQueue itemQueue, ref ItemInfo itemInfo)
    {
        if (itemQueue && itemInfo)
        {
            MoveItemToProcessed(itemQueue, itemInfo);
            Print("[NightroItemGiver] Delayed move to processed completed for: " + itemInfo.classname);
        }
    }

    static void DelayedUpdateJsonFile(string filePath, array<ref ItemInfo> remainingItems, ref ItemQueue itemQueue, PlayerBase player)
    {
        if (filePath != "" && itemQueue)
        {
            UpdateJsonFileWithStatus(filePath, remainingItems, itemQueue, player);
            Print("[NightroItemGiver] Delayed JSON file update completed");
        }
    }
    
    static void CreatePlayerFile(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return;
        
        string steamID = player.GetIdentity().GetPlainId();
        string filePath = IG_PROFILE_FOLDER + IG_PENDING_FOLDER + steamID + ".json";
        
        if (!FileExist(filePath))
        {
            ItemQueue playerQueue = new ItemQueue();
            playerQueue.player_name = player.GetIdentity().GetName();
            playerQueue.steam_id = steamID;
            playerQueue.last_updated = GetCurrentDateTime();
            
            playerQueue.player_state = new PlayerStateInfo();
            playerQueue.player_state.steam_id = steamID;
            
            JsonFileLoader<ItemQueue>.JsonSaveFile(filePath, playerQueue);
            Print("[NightroItemGiver] Created new player file for: " + player.GetIdentity().GetName());
        }
    }
    
    static string GetCurrentDateTime()
    {
        return "updated";
    }
    
    static void UpdateJsonFileWithStatus(string filePath, array<ref ItemInfo> remainingItems, ref ItemQueue itemQueue, PlayerBase player)
    {
        ItemQueue updatedQueue = new ItemQueue();
        
        if (player && player.GetIdentity())
        {
            updatedQueue.player_name = player.GetIdentity().GetName();
            updatedQueue.steam_id = player.GetIdentity().GetPlainId();
            updatedQueue.last_updated = GetCurrentDateTime();
        }
        
        if (remainingItems && remainingItems.Count() > 0)
        {
            updatedQueue.items_to_give = remainingItems;
        }
        
        if (itemQueue.processed_items && itemQueue.processed_items.Count() > 0)
        {
            updatedQueue.processed_items = itemQueue.processed_items;
        }

        if (itemQueue.player_state)
        {
            updatedQueue.player_state = itemQueue.player_state;
        }
        
        JsonFileLoader<ItemQueue>.JsonSaveFile(filePath, updatedQueue);
        Print("[NightroItemGiver] Updated JSON file with status feedback and player state");
        Print("[NightroItemGiver] Remaining items: " + updatedQueue.items_to_give.Count().ToString());
        Print("[NightroItemGiver] Processed items: " + updatedQueue.processed_items.Count().ToString());
    }
    
    static void UpdateJsonFile(string filePath, array<ref ItemInfo> remainingItems, PlayerBase player)
    {
        ItemQueue updatedQueue = new ItemQueue();
        
        if (player && player.GetIdentity())
        {
            updatedQueue.player_name = player.GetIdentity().GetName();
            updatedQueue.steam_id = player.GetIdentity().GetPlainId();
            updatedQueue.last_updated = GetCurrentDateTime();
        }
        
        if (remainingItems && remainingItems.Count() > 0)
        {
            updatedQueue.items_to_give = remainingItems;
        }
        
        JsonFileLoader<ItemQueue>.JsonSaveFile(filePath, updatedQueue);
        Print("[NightroItemGiver] Updated JSON file");
    }
    
    static void ClearJsonFile(string filePath, PlayerBase player)
    {
        ItemQueue emptyQueue = new ItemQueue();
        
        if (player && player.GetIdentity())
        {
            emptyQueue.player_name = player.GetIdentity().GetName();
            emptyQueue.steam_id = player.GetIdentity().GetPlainId();
            emptyQueue.last_updated = GetCurrentDateTime();
        }
        
        JsonFileLoader<ItemQueue>.JsonSaveFile(filePath, emptyQueue);
        Print("[NightroItemGiver] Cleared JSON file");
    }
    
    static void SendNotification(PlayerBase player, string classname, int quantity)
    {
        if (!m_NotificationSettings || !player) return;

        string templateMsg = m_NotificationSettings.message_template;
        templateMsg.Replace("{CLASS_NAME}", classname);
        templateMsg.Replace("{QUANTITY}", quantity.ToString());

        GetGame().ChatMP(player, templateMsg, "");
        Print("[NightroItemGiver] Notification sent: " + templateMsg);
    }
}

modded class PlayerBase
{
    override void OnConnect()
    {
        super.OnConnect();
        Print("===== ENHANCED ITEMGIVER V2.5 WITH ADVANCED TERRITORY VALIDATION AND NOTIFICATION SYSTEM (COMPLETE) =====");
        Print("[NightroItemGiver] Player connected: " + GetIdentity().GetName());
        
        NightroItemGiverManager.UpdatePlayerState(this, "connect");
        NightroItemGiverManager.CreatePlayerFile(this);
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(NightroItemGiverManager.CheckAndGiveItems, 5000, false, this);
    }

    override void EEOnCECreate()
    {
        super.EEOnCECreate();
        
        if (GetIdentity())
        {
            Print("[NightroItemGiver] Player respawned: " + GetIdentity().GetName());
            NightroItemGiverManager.UpdatePlayerState(this, "respawn");
        }
    }
}

static ref NightroItemGiverManager g_NightroItemGiverManager = new NightroItemGiverManager();