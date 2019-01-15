#include "main.h"

static SlrxFilterIdMask_t FiltersIdMask;

int addBeaconRxFilter()
{
    /**
     * We are creating two decision tree:
     * 1. FRAME_TYPE != 'MANAGEMENT' then DROP
     * 2. FRAME_TYPE == 'MANAGEMENT' -> FRAME_TYPE != BEACON then DROP
     */

    SlrxFilterRuleType_t RuleType;
    SlrxFilterID_t FilterId = 0;
    SlrxFilterFlags_t FilterFlags;

    SlrxFilterRule_t Rule;
    SlrxFilterTrigger_t Trigger;
    SlrxFilterAction_t Action;

    uint8_t FrameType;
    uint8_t FrameSubtype;
    uint8_t FrameTypeMask;

    memset(FiltersIdMask, 0, sizeof(FiltersIdMask));

    RuleType = HEADER;
    FilterFlags.IntRepresentation = RX_FILTER_BINARY;
    FrameType = TYPE_MANAGEMENT;
    FrameTypeMask = 0xFF;

    Rule.HeaderType.RuleHeaderfield = FRAME_TYPE_FIELD;
    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB1BytesRuleArgs[0],
            &FrameType, 1);
    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask, &FrameTypeMask, 1);
    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_NOT_EQUAL_TO;

    Trigger.ParentFilterID = 0;
    Trigger.Trigger = NO_TRIGGER;
    Trigger.TriggerArgConnectionState.IntRepresentation =
    RX_FILTER_CONNECTION_STATE_STA_NOT_CONNECTED;
    Trigger.TriggerArgRoleStatus.IntRepresentation = RX_FILTER_ROLE_PROMISCUOUS;

    Action.ActionType.IntRepresentation = RX_FILTER_ACTION_DROP;

    int16_t retVal = sl_WlanRxFilterAdd(RuleType, FilterFlags, &Rule, &Trigger, &Action, &FilterId);
    if (retVal != 0) {
        DEBUG("[ERROR] Can not add filter: %d", retVal);
        return -1;
    }
    DEBUG("Filter created, id: %d", FilterId);

    SETBIT8(FiltersIdMask, FilterId);

    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_EQUAL;
    Action.ActionType.IntRepresentation = RX_FILTER_ACTION_NULL;

    retVal = sl_WlanRxFilterAdd(RuleType, FilterFlags, &Rule, &Trigger, &Action, &FilterId);
    if (retVal != 0) {
        DEBUG("[ERROR] Can not add filter: %d", retVal);
        return -1;
    }
    DEBUG("Filter created, id: %d", FilterId);

    SETBIT8(FiltersIdMask, FilterId);

    Trigger.ParentFilterID = FilterId;
    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_NOT_EQUAL_TO;
    Action.ActionType.IntRepresentation = RX_FILTER_ACTION_DROP;

    // RxFilter expects the sub-type in the second 4-bits group (0x80 for beacon frame)
    FrameSubtype = (MGM_SUBTYPE_BEACON << 4);
    Rule.HeaderType.RuleHeaderfield = FRAME_SUBTYPE_FIELD;
    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB1BytesRuleArgs[0],
            &FrameSubtype, 1);

    retVal = sl_WlanRxFilterAdd(RuleType, FilterFlags, &Rule, &Trigger, &Action, &FilterId);
    if (retVal != 0) {
        DEBUG("[ERROR] Can not add filter: %d", retVal);
        return -1;
    }
    DEBUG("Filter created, id: %d", FilterId);

    SETBIT8(FiltersIdMask, FilterId);

    return 0;
}

int enableBeaconRxFilter()
{
    SlrxFilterIdMask_t emptyFiltersIdMask = { 0, };
    if (memcmp(emptyFiltersIdMask, FiltersIdMask, sizeof(SlrxFilterIdMask_t)) == 0) {
        DEBUG("[ERROR] beacon filter was not created");
        return -1;
    }

    _WlanRxFilterOperationCommandBuff_t filterOperation;
    memset(&filterOperation, 0, sizeof(filterOperation));
    memcpy(filterOperation.FilterIdMask, FiltersIdMask, sizeof(FiltersIdMask));

    _i16 retVal = sl_WlanRxFilterSet(SL_ENABLE_DISABLE_RX_FILTER, &filterOperation,
            sizeof(filterOperation));
    if (retVal != 0) {
        DEBUG("[ERROR]sl_WlanRxFilterSet, retVal: %d", retVal);
        return -1;
    }
    return 0;
}

int disableBeaconRxFilter()
{
    SlrxFilterIdMask_t emptyFiltersIdMask = { 0, };
    if (memcmp(emptyFiltersIdMask, FiltersIdMask, sizeof(SlrxFilterIdMask_t)) == 0) {
        DEBUG("[ERROR] beacon filter was not created");
        return -1;
    }

    _WlanRxFilterOperationCommandBuff_t filterOperation;
    memset(&filterOperation, 0, sizeof(filterOperation));
    memcpy(filterOperation.FilterIdMask, emptyFiltersIdMask, sizeof(emptyFiltersIdMask));

    _i16 retVal = sl_WlanRxFilterSet(SL_ENABLE_DISABLE_RX_FILTER, &filterOperation,
            sizeof(filterOperation));
    if (retVal != 0) {
        DEBUG("[ERROR]sl_WlanRxFilterSet, retVal: %d", retVal);
        return -1;
    }
    return 0;
}

int printRxFilterMask()
{
    printf("\n## Rx Filters (sl_WlanRxFilterGet)\n");

    _WlanRxFilterRetrieveEnableStatusCommandResponseBuff_t buf;
    _i16 retVal = sl_WlanRxFilterGet(SL_FILTER_RETRIEVE_ENABLE_STATE, &buf,
            sizeof(buf));
    if (retVal < 0) {
        DEBUG("Failed sl_WlanRxFilterGet: %d", retVal);
        return -1;
    }

    printf("Enabled Filters: \n");
    printf("\t%08X\n", ((_u32*) &buf.FilterIdMask)[0]);
    printf("\t%08X\n", ((_u32*) &buf.FilterIdMask)[1]);
    printf("\t%08X\n", ((_u32*) &buf.FilterIdMask)[2]);
    printf("\t%08X\n", ((_u32*) &buf.FilterIdMask)[3]);
    return 0;
}
