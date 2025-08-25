class CfgPatches
{
    class Nightro_Giveitem_Scripts
    {
        units[]={};
        weapons[]={};
        requiredVersion=0.1;
        requiredAddons[]=
        {
            "DZ_Data",
            "DZ_Scripts",
            "LBmaster_Core",
            "LBmaster_Groups"
        };
    };
};

class CfgMods
{
    class Nightro_Giveitem
    {
        dir="Nightro_Giveitem";
        name="Nightro Give Item with Advanced Territory Check";
        version="2.5";
        type="mod";
        author="Nightro";
        dependencies[]={"World", "LBmaster_Groups"};
        
        defines[] = {
            "NIGHTRO_ITEMGIVER_TERRITORY_CHECK",
            "NIGHTRO_ITEMGIVER_ADVANCED_NOTIFICATIONS"
        };
        
        class defs
        {
            class worldScriptModule
            {
                value="";
                files[]=
                {
                    "LBmaster_Groups/scripts/4_World",
                    "Nightro_Giveitem/scripts/4_World"
                };
            };
        };
    };
};