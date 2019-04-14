const ZigbeeCommand = {
    OnOff: {
        On: 'zcl on-off on',
        Off: 'zcl on-off off',
        Toggle: 'zcl on-off toggle',
    },
    Plugin: {
        FormNetwork: 'plugin network-creator start',
        LeaveNetwork: 'network leave',
        PermitJoin: 'plugin network-creator-security open-network',
        PermitStopJoin: 'plugin network-creator-security close-network',
        DeviceTable: 'plugin device-table send'
    },
    LevelControl: {
        MoveToLevel: 'zcl level-control mv-to-level',
        Move: 'zcl level-control move',
        Stop: 'zcl level-control stop',
    }
};

const ZigbeeCluster = {
    BASIC: {
        ID: '0x0000',
        Attribute: {
            ZCL_VERSION_ATTRIBUTE_ID: { ID: '0002', type: '20' },
            ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID: { ID: '0004', type: '42' },
            ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID: { ID: '0005', type: '42' },
            ZCL_POWER_SOURCE_ATTRIBUTE_ID: { ID: '0007', type: '30' },
            ZCL_BASIC_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: { ID: 'FFFD', type: '21' }
        }
    },
    ON_OFF: {
        ID: '0x0006',
        Attribute: {
            ZCL_ON_OFF_ATTRIBUTE_ID: { ID: '0000',  type: '10' },
            ZCL_ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: { ID: 'FFFD', type: '21' }
        },
        Command: {
            On: '01',
            Off: '00',
            Toggle: '02',
            OffWithEffect: '40',
            OnWithTimeOff: '42'
        }
    },
    IDENTIFY: {
        ID: '0x0003',
        Attribute: {
            ZCL_IDENTIFY_TIME_ATTRIBUTE_ID: { ID: '0000', type: '21' },
            ZCL_COMMISSION_STATE_ATTRIBUTE_ID: { ID: '0001', type: '18' },
            ZCL_IDENTIFY_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: { ID: 'FFFD', type: '21' }
        },
        Command: {
            Identify: '00',
            IdentifyQuery: '01'
        }
    },
    LEVEL_CONTROL: {
        ID: '0x0008',
        Attribute: {
            ZCL_CURRENT_LEVEL_ATTRIBUTE_ID: { ID: '0000', type: '20' },
            ZCL_LEVEL_CONTROL_REMAINING_TIME_ATTRIBUTE_ID: { ID: '0001', type: '21' },
            ZCL_ON_LEVEL_ATTRIBUTE_ID: { ID: '0011', type: '20' },
            ZCL_LEVEL_CONTROL_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: { ID: 'FFFD', type: '21' }
        },
        Command: {
            MoveToLevel: '00',
            Move: '01',
            Step: '02',
            Stop: '03',
            MoveToLevelWithOnOff: '04',
            MoveWithOnOff: '05',
            StepWithOnOff: '06',
            StopWithOnOff: '07'
        }
    },
    LUMINANCE_MEASUREMENT: {
        ID: '0x0400',
        Attribute: {

        }
    },
    TEMPERATURE_MEASUREMENT: {
        ID: '0x0402',
        Attribute: {

        }
    },
    HUMIDITY_MEASUREMENT: {
        ID: '0x0405',
        Attribute: {

        }
    },
    IAS_ZONE: {
        ID: '0x0500',
        Attribute: {

        }
    },
    DOOR_LOCK: {
        ID: '0x0101',
        Attribute: {

        }
    }
};

module.exports = { ZigbeeCommand, ZigbeeCluster};