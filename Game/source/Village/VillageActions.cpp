#include "Village/VillageActions.h"

#include "FreeCamera.h"
#include "Village/VillageController.h"
#include "Village/Map/Editor.h"
#include "UI/UiCanvas.h"

namespace GPC
{
    void ActionToggleScroll::Start(VillageController* pOwner)
    {
        // TODO Removed
        GPC_INFO << "Scroll : " << Enable << ENDL;
    }

    void ActionToggleMainUI::Start(VillageController* pOwner)
    {
        if (pOwner->GetMainUI())
        {
            pOwner->GetMainUI()->IsEnable = Enable;
        }
    }

    void ActionToggleEditUI::Start(VillageController* pOwner)
    {
        if (pOwner->GetEditor())
        {
            pOwner->GetEditor()->SetPlannerActive(Enable);
            pOwner->GetEditor()->SetFullUIActive(Enable);
        }

        if (pOwner->GetEditUI())
        {
            pOwner->GetEditUI()->IsEnable = Enable;
        }
    }

    void ActionToggleInspectUI::Start(VillageController* pOwner)
    {
        if (pOwner->GetInspectUI())
        {
            pOwner->GetInspectUI()->IsEnable = Enable;
        }
    }

    void ActionToggleBuildingEdit::Start(VillageController* pOwner)
    {
        // Todo : Remplacer par l'UI
        if (Enable)
            GPC_INFO << "Building Edit Enabled" << ENDL;
        else
            GPC_INFO << "Building Edit Disabled" << ENDL;
    }

    void ActionToggleBuildingInteract::Start(VillageController *pOwner)
    {
        // Todo : Remplacer par l'UI
        if (Enable)
            GPC_INFO << "Building Interact Enabled" << ENDL;
        else
            GPC_INFO << "Building Interact Disabled" << ENDL;
    }
}