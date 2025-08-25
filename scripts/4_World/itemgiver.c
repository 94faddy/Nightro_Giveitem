// ===== ENHANCED ITEMGIVER V2.5 WITH ADVANCED TERRITORY VALIDATION AND NOTIFICATION SYSTEM (PRODUCTION) =====

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
        Init();
    }
    
    static void Init()
    {
        if (m_IsInitialized) return;
        
        if (!FileExist(IG_PROFILE_FOLDER)) 
        {
            MakeDirectory(IG_PROFILE_FOLDER);
        }
        
        if (!FileExist(IG_PROFILE_FOLDER + IG_PENDING_FOLDER)) 
        {
            MakeDirectory(IG_PROFILE_FOLDER + IG_PENDING_FOLDER);
        }

        m_PlayerStates = new map<string, ref PlayerStateInfo>;

        LoadNotificationSettings();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckAllOnlinePlayers, IG_CHECK_INTERVAL, true);
        
        m_IsInitialized = true;
    }
    
    static void LoadNotificationSettings()
    {
        string settingsPath = IG_PROFILE_FOLDER + IG_SETTINGS_FILE;
        if (FileExist(settingsPath))
        {
            JsonFileLoader<NotificationSettings>.JsonLoadFile(settingsPath, m_NotificationSettings);
        }
        else
        {
            m_NotificationSettings = new NotificationSettings();
            JsonFileLoader<NotificationSettings>.JsonSaveFile(settingsPath, m_NotificationSettings);
        }
    }

    static bool HasPendingItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return false;
        
        string steamID = player.GetIdentity().GetPlainId();
        string filePath = IG_PROFILE_FOLDER + IG_PENDING_FOLDER + steamID + ".json";
        
        if (!FileExist(filePath)) 
        {
            return false;
        }
        
        ItemQueue itemQueue = new ItemQueue();
        JsonFileLoader<ItemQueue>.JsonLoadFile(filePath, itemQueue);
        
        if (!itemQueue || !itemQueue.items_to_give) 
        {
            return false;
        }
        
        for (int i = 0; i < itemQueue.items_to_give.Count(); i++)
        {
            ItemInfo item = itemQueue.items_to_give.Get(i);
            if (item && item.classname != "" && item.quantity > 0)
            {
                return true;
            }
        }
        
        return false;
    }

    static bool IsPlayerInValidTerritory(PlayerBase player, out float distanceToPlotpole)
    {
        distanceToPlotpole = -1;
        
        if (!player || !player.GetIdentity()) 
        {
            return false;
        }

        string steamID = player.GetIdentity().GetPlainId();

        // Check if territory validation is enabled
        if (!m_NotificationSettings.require_territory_check)
        {
            return true;
        }

        bool hasPendingItems = HasPendingItems(player);
        
        if (!hasPendingItems)
        {
            return false; 
        }

        LBGroup playerGroup = GetPlayerGroup(player);
        if (!playerGroup)
        {
            distanceToPlotpole = GetDistanceToNearestPlotpole(player.GetPosition());
            
            string noGroupMsg = m_NotificationSettings.no_group_message;
            noGroupMsg.Replace("{DISTANCE}", ((int)distanceToPlotpole).ToString());
            SendPeriodicNotification(player, noGroupMsg, "no_group", m_NotificationSettings.no_group_notification_interval);
            return false;
        }

        if (!GroupHasPlotpole(playerGroup))
        {
            SendPeriodicNotification(player, m_NotificationSettings.no_plotpole_message, "no_plotpole", m_NotificationSettings.no_plotpole_notification_interval);
            return false;
        }

        bool isInTerritory = IsPlayerInGroupTerritory(player, playerGroup, distanceToPlotpole);
        if (!isInTerritory)
        {
            string territoryMsg = m_NotificationSettings.outside_territory_message;
            territoryMsg.Replace("{DISTANCE}", ((int)distanceToPlotpole).ToString());
            SendPeriodicNotification(player, territoryMsg, "outside_territory", m_NotificationSettings.outside_territory_notification_interval);
            return false;
        }

        return true;
    }

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

    static LBGroup GetPlayerGroup(PlayerBase player)
    {
        if (!player)
            return null;
            
        return player.GetLBGroup();
    }

    static bool IsPlayerInGroupTerritory(PlayerBase player, LBGroup playerGroup, out float distanceToNearestFriendlyFlag)
    {
        distanceToNearestFriendlyFlag = 999999.0;
        
        if (!player || !playerGroup) 
            return false;

        vector playerPos = player.GetPosition();
        string groupTag = playerGroup.shortname;

        #ifdef LBmaster_GroupDLCPlotpole
        string groupTagLower = groupTag;
        groupTagLower.ToLower();
        int groupTagHash = groupTagLower.Hash();
        
        TerritoryFlag nearestFriendlyFlag = TerritoryFlag.FindNearestFlag(playerPos, true, true, groupTagHash);
        
        if (nearestFriendlyFlag)
        {
            distanceToNearestFriendlyFlag = vector.Distance(playerPos, nearestFriendlyFlag.GetPosition());
            
            if (nearestFriendlyFlag.IsInRadius(playerPos))
            {
                return true;
            }
        }
        
        return false;
        #else
        return true;
        #endif
    }

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
        
        bool shouldNotify = false;
        if (playerState.last_notification_type != notificationType)
        {
            shouldNotify = true;
        }
        else if ((currentTime - playerState.last_notification_time) >= (intervalSeconds * 1000))
        {
            shouldNotify = true;
        }
        
        if (shouldNotify)
        {
            GetGame().ChatMP(player, notificationMessage, "ColorRed");
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
        
        string steamID = player.GetIdentity().GetPlainId();
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
        }
    }

    static void SendInventoryFullNotification(PlayerBase player, string itemName, int quantity)
    {
        if (!player || !player.GetIdentity()) return;

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
        
        string steamID = player.GetIdentity().GetPlainId();
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
        }
    }

    static bool IsPlayerReadyForItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return false;
        
        string steamID = player.GetIdentity().GetPlainId();
        
        if (player.IsUnconscious() || player.IsRestrained() || player.IsAlive() == false)
        {
            return false;
        }
        
        if (player.IsInVehicle() && player.GetParent())
        {
            return false;
        }

        float distanceToPlotpole;
        if (!IsPlayerInValidTerritory(player, distanceToPlotpole))
        {
            return false;
        }
        
        bool hasPendingItems = HasPendingItems(player);
        
        if (!hasPendingItems)
        {
            return false;
        }
        
        if (m_PlayerStates.Contains(steamID))
        {
            PlayerStateInfo playerState = m_PlayerStates.Get(steamID);
            float currentTime = GetGame().GetTime();
            
            float timeSinceConnect = currentTime - playerState.last_connect_time;
            if (timeSinceConnect < m_NotificationSettings.initial_delay_seconds * 1000)
            {
                string waitingMsg = m_NotificationSettings.waiting_conditions_message;
                SendPeriodicNotification(player, waitingMsg, "waiting_conditions", m_NotificationSettings.waiting_conditions_notification_interval);
                return false;
            }
            
            if (playerState.last_respawn_time > 0)
            {
                float timeSinceRespawn = currentTime - playerState.last_respawn_time;
                if (timeSinceRespawn < m_NotificationSettings.respawn_delay_seconds * 1000)
                {
                    string respawnWaitingMsg = m_NotificationSettings.waiting_conditions_message;
                    SendPeriodicNotification(player, respawnWaitingMsg, "waiting_conditions", m_NotificationSettings.waiting_conditions_notification_interval);
                    return false;
                }
            }
            
            playerState.is_ready_for_items = true;
        }
        
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
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
            
            if (!player.IsAlive())
            {
                playerState.was_dead_on_connect = true;
            }
            else
            {
                playerState.was_dead_on_connect = false;
            }
        }
        else if (action == "respawn")
        {
            playerState.last_respawn_time = currentTime;
            playerState.is_ready_for_items = false;
            playerState.was_dead_on_connect = false;
            playerState.last_notification_time = 0;
            playerState.last_notification_type = "";
        }
    }
    
    static void CheckAllOnlinePlayers()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            Man p = players.Get(i);
            PlayerBase player = PlayerBase.Cast(p);
            if (player && player.GetIdentity())
            {
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
    }

    static void UpdateAttachmentStatus(ref AttachmentInfo attachment, string status)
    {
        attachment.status = status;
    }

    static void UpdateItemRetry(ref ItemInfo itemInfo, string errorMessage = "")
    {
        itemInfo.retry_count++;
        itemInfo.last_retry_at = GetGame().GetTime().ToString();
        itemInfo.error_message = errorMessage;
    }

    static bool ShouldRetryItem(ref ItemInfo itemInfo)
    {
        if (itemInfo.retry_count >= m_NotificationSettings.max_retry_attempts)
        {
            return false;
        }
        
        if (itemInfo.last_retry_at == "" || itemInfo.retry_count == 0)
        {
            return true;
        }
        
        float currentTime = GetGame().GetTime();
        float lastRetryTime = itemInfo.last_retry_at.ToFloat();
        float timeSinceLastRetry = currentTime - lastRetryTime;
        float retryIntervalMs = m_NotificationSettings.retry_interval_seconds * 1000;
        
        if (timeSinceLastRetry >= retryIntervalMs)
        {
            return true;
        }
        
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
                return false;
            }

            InventoryLocation invLocation = new InventoryLocation();
            bool canFit = player.GetInventory().FindFreeLocationFor(testItem, FindInventoryLocationType.ANY, invLocation);
            GetGame().ObjectDelete(testItem);
            
            if (!canFit)
            {
                return false;
            }
        }

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
        
        Magazine currentMag = weapon.GetMagazine(0);
        if (currentMag)
        {
            string magType = currentMag.GetType();
            
            array<string> validMags = {};
            GetGame().ConfigGetTextArray("CfgWeapons " + weapon.GetType() + " magazines", validMags);
            
            if (validMags.Find(magType) == -1)
            {
                weapon.ServerDropEntity(currentMag);
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().ObjectDelete, 1000, false, currentMag);
            }
            else
            {
                currentMag.ServerSetAmmoCount(currentMag.GetAmmoMax());
            }
        }
        
        weapon.ValidateAndRepair();
        weapon.SetSynchDirty();
        weapon.Synchronize();
    }
    
    static void AttachItemsToWeapon(EntityAI parentItem, array<ref AttachmentInfo> attachments, PlayerBase player)
    {
        if (!parentItem || !attachments || !player) return;
        
        for (int i = 0; i < attachments.Count(); i++)
        {
            AttachmentInfo attachment = attachments.Get(i);
            if (!attachment || attachment.classname == "") continue;
            
            if (!IsValidItemClass(attachment.classname))
            {
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
                            weapon.ServerDropEntity(existingMag);
                            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().ObjectDelete, 500, false, existingMag);
                        }
                        
                        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(InstallMagazineDelayed, 800, false, weapon, attachment.classname, attachment);
                        attachmentSuccess = true;
                    }
                    else
                    {
                        EntityAI playerMagItem = player.GetInventory().CreateInInventory(attachment.classname);
                        if (playerMagItem)
                        {
                            Magazine mag = Magazine.Cast(playerMagItem);
                            if (mag) mag.ServerSetAmmoCount(mag.GetAmmoMax());
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
                        UpdateAttachmentStatus(attachment, "delivered");
                        attachmentSuccess = true;
                        
                        if (attachment.attachments && attachment.attachments.Count() > 0)
                        {
                            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AttachItemsToWeapon, 500, false, attachmentEntity, attachment.attachments, player);
                        }
                    }
                    else
                    {
                        EntityAI playerAttachmentItem = player.GetInventory().CreateInInventory(attachment.classname);
                        if (playerAttachmentItem)
                        {
                            UpdateAttachmentStatus(attachment, "delivered");
                            attachmentSuccess = true;
                            
                            if (attachment.attachments && attachment.attachments.Count() > 0)
                            {
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
        }
    }
    
    static void InstallMagazineDelayed(Weapon_Base weapon, string magazineClass, ref AttachmentInfo attachment)
    {
        if (!weapon || magazineClass == "" || !attachment) return;
        
        Magazine existingMag = weapon.GetMagazine(0);
        if (existingMag)
        {
            return;
        }
        
        EntityAI newMag = weapon.GetInventory().CreateAttachment(magazineClass);
        if (newMag)
        {
            Magazine magazine = Magazine.Cast(newMag);
            if (magazine)
            {
                magazine.ServerSetAmmoCount(magazine.GetAmmoMax());
                UpdateAttachmentStatus(attachment, "delivered");
                
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ValidateWeaponState, 500, false, weapon);
            }
        }
        else
        {
            UpdateAttachmentStatus(attachment, "failed");
        }
    }
    
    static void CheckAndGiveItems(PlayerBase player)
    {
        if (!player || !player.GetIdentity()) return;

        string steamID = player.GetIdentity().GetPlainId();
        string filePath = IG_PROFILE_FOLDER + IG_PENDING_FOLDER + steamID + ".json";

        if (!FileExist(filePath))
        {
            return;
        }
        
        ItemQueue itemQueue = new ItemQueue();
        JsonFileLoader<ItemQueue>.JsonLoadFile(filePath, itemQueue);
        
        if (!itemQueue)
        {
            ClearJsonFile(filePath, player);
            return;
        }
        
        if (!itemQueue.items_to_give)
        {
            itemQueue.items_to_give = new array<ref ItemInfo>;
            return;
        }
        
        if (!itemQueue.processed_items)
        {
            itemQueue.processed_items = new array<ref ProcessedItemInfo>;
        }

        if (!itemQueue.player_state)
        {
            itemQueue.player_state = new PlayerStateInfo();
            itemQueue.player_state.steam_id = steamID;
        }
        
        if (itemQueue.items_to_give.Count() > 0)
        {
            array<ref ItemInfo> itemsToProcess = new array<ref ItemInfo>;
            array<ref ItemInfo> remainingItems = new array<ref ItemInfo>;
            
            bool playerReadyForDelivery = IsPlayerReadyForItems(player);
            
            for (int itemIndex = 0; itemIndex < itemQueue.items_to_give.Count(); itemIndex++)
            {
                ItemInfo currentItem = itemQueue.items_to_give.Get(itemIndex);
                
                if (currentItem && currentItem.classname != "" && currentItem.quantity > 0)
                {
                    if (currentItem.status == "pending" || currentItem.status == "")
                    {
                        if (playerReadyForDelivery)
                        {
                            itemsToProcess.Insert(currentItem);
                        }
                        else
                        {
                            if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                            {
                                UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - player not ready for delivery");
                                MoveItemToProcessed(itemQueue, currentItem);
                                SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                            }
                            else
                            {
                                UpdateItemRetry(currentItem, "Player not ready for delivery");
                                currentItem.status = "territory_failed";
                                remainingItems.Insert(currentItem);
                            }
                        }
                    }
                    else if ((currentItem.status == "inventory_full" || currentItem.status == "territory_failed") && ShouldRetryItem(currentItem))
                    {
                        if (playerReadyForDelivery)
                        {
                            itemsToProcess.Insert(currentItem);
                        }
                        else
                        {
                            if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                            {
                                UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - player not ready for delivery");
                                MoveItemToProcessed(itemQueue, currentItem);
                                SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                            }
                            else
                            {
                                UpdateItemRetry(currentItem, "Player still not ready for delivery");
                                remainingItems.Insert(currentItem);
                            }
                        }
                    }
                    else if (currentItem.status == "inventory_full" || currentItem.status == "territory_failed")
                    {
                        if (currentItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                        {
                            UpdateItemStatus(currentItem, "failed", "Max retry attempts reached - " + currentItem.error_message);
                            MoveItemToProcessed(itemQueue, currentItem);
                            SendMaxRetryReachedNotification(player, currentItem.classname, currentItem.quantity);
                        }
                        else
                        {
                            remainingItems.Insert(currentItem);
                        }
                    }
                    else
                    {
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
                
                if (!IsValidItemClass(processItem.classname))
                {
                    UpdateItemStatus(processItem, "failed", "Invalid classname: " + processItem.classname);
                    MoveItemToProcessed(itemQueue, processItem);
                    continue;
                }

                // Final territory check before giving items
                float distanceToPlotpole;
                if (!IsPlayerInValidTerritory(player, distanceToPlotpole))
                {
                    UpdateItemRetry(processItem, "Player not in valid territory");
                    
                    if (processItem.retry_count >= m_NotificationSettings.max_retry_attempts)
                    {
                        UpdateItemStatus(processItem, "failed", "Max retry attempts reached - territory validation failed");
                        MoveItemToProcessed(itemQueue, processItem);
                        SendMaxRetryReachedNotification(player, processItem.classname, processItem.quantity);
                    }
                    else
                    {
                        processItem.status = "territory_failed";
                        remainingItems.Insert(processItem);
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
                            
                            if (processItem.attachments && processItem.attachments.Count() > 0)
                            {
                                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(AttachItemsToWeapon, 300, false, item, processItem.attachments, player);
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    
                    if (itemsGiven > 0)
                    {
                        SendSuccessNotification(player, processItem.classname, itemsGiven);
                        
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
                    }
                    else
                    {
                        processItem.status = "inventory_full";
                        remainingItems.Insert(processItem);
                        SendInventoryFullNotification(player, processItem.classname, processItem.quantity);
                    }
                }
            }
            
            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DelayedUpdateJsonFile, 6000, false, filePath, remainingItems, itemQueue, player);
        }
    }

    static void DelayedMoveToProcessed(ref ItemQueue itemQueue, ref ItemInfo itemInfo)
    {
        if (itemQueue && itemInfo)
        {
            MoveItemToProcessed(itemQueue, itemInfo);
        }
    }

    static void DelayedUpdateJsonFile(string filePath, array<ref ItemInfo> remainingItems, ref ItemQueue itemQueue, PlayerBase player)
    {
        if (filePath != "" && itemQueue)
        {
            UpdateJsonFileWithStatus(filePath, remainingItems, itemQueue, player);
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
    }
    
    static void SendNotification(PlayerBase player, string classname, int quantity)
    {
        if (!m_NotificationSettings || !player) return;

        string templateMsg = m_NotificationSettings.message_template;
        templateMsg.Replace("{CLASS_NAME}", classname);
        templateMsg.Replace("{QUANTITY}", quantity.ToString());

        GetGame().ChatMP(player, templateMsg, "");
    }
}

modded class PlayerBase
{
    override void OnConnect()
    {
        super.OnConnect();
        
        NightroItemGiverManager.UpdatePlayerState(this, "connect");
        NightroItemGiverManager.CreatePlayerFile(this);
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(NightroItemGiverManager.CheckAndGiveItems, 5000, false, this);
    }

    override void EEOnCECreate()
    {
        super.EEOnCECreate();
        
        if (GetIdentity())
        {
            NightroItemGiverManager.UpdatePlayerState(this, "respawn");
        }
    }
}

static ref NightroItemGiverManager g_NightroItemGiverManager = new NightroItemGiverManager();