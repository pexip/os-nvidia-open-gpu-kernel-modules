// Generated by the protocol buffer compiler.  DO NOT EDIT!

#include "nvtypes.h"
#include "prbrt.h"
#include "g_all_dcl_pb.h"

// 'Engines' field defaults

// 'Engines' field descriptors
const PRB_FIELD_DESC prb_fields_dcl_engines[] = {
    {
        1,
        {
            PRB_REPEATED,
            PRB_MESSAGE,
            0,
        },
        NVDEBUG_ENG_GPU,
        0,
        PRB_MAYBE_FIELD_NAME("eng_gpu")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        2,
        {
            PRB_REPEATED,
            PRB_MESSAGE,
            0,
        },
        NVDEBUG_ENG_MC,
        0,
        PRB_MAYBE_FIELD_NAME("eng_mc")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
};

// 'DclMsg' field defaults

// 'DclMsg' field descriptors
const PRB_FIELD_DESC prb_fields_dcl_dclmsg[] = {
    {
        330,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        JOURNAL_COMMON,
        0,
        PRB_MAYBE_FIELD_NAME("common")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        315,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        JOURNAL_ASSERT,
        0,
        PRB_MAYBE_FIELD_NAME("journal_assert")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        320,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        JOURNAL_RVAHEADER,
        0,
        PRB_MAYBE_FIELD_NAME("journal_rvaheader")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        321,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        JOURNAL_BADREAD,
        0,
        PRB_MAYBE_FIELD_NAME("journal_badread")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        327,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        JOURNAL_BUGCHECK,
        0,
        PRB_MAYBE_FIELD_NAME("journal_bugcheck")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        329,
        {
            PRB_REPEATED,
            PRB_MESSAGE,
            0,
        },
        RC_RCCOUNTER,
        0,
        PRB_MAYBE_FIELD_NAME("rcCounter")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
    {
        3,
        {
            PRB_OPTIONAL,
            PRB_MESSAGE,
            0,
        },
        DCL_ENGINES,
        0,
        PRB_MAYBE_FIELD_NAME("engine")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
};

// 'ErrorBlock' field defaults

// 'ErrorBlock' field descriptors
const PRB_FIELD_DESC prb_fields_dcl_errorblock[] = {
    {
        1,
        {
            PRB_REPEATED,
            PRB_MESSAGE,
            0,
        },
        DCL_DCLMSG,
        0,
        PRB_MAYBE_FIELD_NAME("data")
        PRB_MAYBE_FIELD_DEFAULT(0)
    },
};

// Message descriptors
const PRB_MSG_DESC prb_messages_dcl[] = {
    {
        2,
        prb_fields_dcl_engines,
        PRB_MAYBE_MESSAGE_NAME("Dcl.Engines")
    },
    {
        7,
        prb_fields_dcl_dclmsg,
        PRB_MAYBE_MESSAGE_NAME("Dcl.DclMsg")
    },
    {
        1,
        prb_fields_dcl_errorblock,
        PRB_MAYBE_MESSAGE_NAME("Dcl.ErrorBlock")
    },
};

// Service descriptors
const PRB_SERVICE_DESC prb_services_dcl[] = {
   { 0 }
};

