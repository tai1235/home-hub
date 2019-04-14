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
            VERSION: { ID: '0002', type: '20' },
            MANUFACTURER_NAME: { ID: '0004', type: '42' },
            MODEL_IDENTIFIER: { ID: '0005', type: '42' },
            POWER_SOURCE: { ID: '0007', type: '30' },
            BASIC_CLUSTER_CLUSTER_REVISION_SERVER: { ID: 'FFFD', type: '21' }
        }
    },
    ON_OFF: {
        ID: '0x0006',
        Attribute: {
            ON_OFF: { ID: '0000',  type: '10' },
            ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER: { ID: 'FFFD', type: '21' }
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
            IDENTIFY_TIME: { ID: '0000', type: '21' },
            COMMISSION_STATE: { ID: '0001', type: '18' },
            IDENTIFY_CLUSTER_CLUSTER_REVISION_SERVER: { ID: 'FFFD', type: '21' }
        },
        Command: {
            Identify: '00',
            IdentifyQuery: '01'
        }
    },
    LEVEL_CONTROL: {
        ID: '0x0008',
        Attribute: {
            CURRENT_LEVEL: { ID: '0000', type: '20' },
            LEVEL_CONTROL_REMAINING_TIME: { ID: '0001', type: '21' },
            ON_LEVEL: { ID: '0011', type: '20' },
            LEVEL_CONTROL_CLUSTER_CLUSTER_REVISION_SERVER: { ID: 'FFFD', type: '21' }
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