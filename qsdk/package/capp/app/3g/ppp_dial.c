#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hi_3g_intf.h"

#define MAX_CMD_RETRIES        5

#define CMD_SUPPORT_LITE       0 /* for CDMA cards */
#define CMD_SUPPORT_FULL       1 /* for WCDMA cards */

#if 0
#  define DBG_OUT(fmt, arg...) \
    printf("  %s[%d]: " fmt, __FUNCTION__, __LINE__, ##arg)
#else
#  define DBG_OUT(fmt, arg...)
#endif

static int max_operator_num = MAX_OPERATOR_NUM;
static HI_3G_CARD_S *card;
static HI_3G_OPERATOR_S *operator;
static HI_3G_OPERATOR_S *operatororg;
static HI_3G_CARD_OPERATOR_S *currentoperator;
static HI_3G_CARD_FLOW_S *dataflow;

static int set_register_operator(HI_3G_CARD_MODE_E search_mode, 
				 HI_3G_CARD_ACQORDER_E acqorder, 
				 HI_3G_CARD_COPS_MODE_E register_mode, 
				 HI_3G_CARD_COPS_OP_FORMAT_E register_format)
{
	int ret, ix;
	int operator_num = 0;
	char register_op_name[32] = {0};

	if ((search_mode >= HI_3G_CARD_MODE_BOTT) 
	   || ((search_mode < HI_3G_CARD_MODE_GSM_ONLY) 
	   && (search_mode != HI_3G_CARD_MODE_AUTO))) {
		printf("input search_mode invalid(%d)!\n", search_mode);
		return HI_3G_CARD_ERR_INVAL;
	}
	if (acqorder >= HI_3G_CARD_ACQORDER_BOTT) {
		printf("input acqorder invalid(%d)!\n", acqorder);
		return HI_3G_CARD_ERR_INVAL;
	}
	if (register_mode >= HI_3G_CARD_COPS_MODE_BOTT) {
		printf("input register mode invlaid\n");
		return HI_3G_CARD_ERR_INVAL;
	}
	if (register_format >= HI_3G_CARD_COPS_OP_FORMAT_BOTT) {
		printf("input register format invlaid(%d)!\n", 
			register_format);
		return HI_3G_CARD_ERR_INVAL;
	}
	ret = HI_3G_SearchOperatorModeAcqorder(card, search_mode, acqorder);
	if (ret != 0) {
		DBG_OUT("set search mode(%d) and acqorder(%d) failed(%d)\n", 
			search_mode, acqorder, ret);
		return ret;
	}

	ret = HI_3G_GetAllOperators (card, 
				     operator, 
				     max_operator_num, 
				     &operator_num);
	
	if (ret != 0) {
		DBG_OUT("get all operators failed(%d)!\n", ret);
		return ret;
	}
	printf("get %d operators successfully:\n", operator_num);
	for (ix = 0; ix < operator_num; ix++) {
	printf("operator[%d]:\n"
		"	netmode=%s\n"
		"	long string=%s\n"
		"	short string=%s\n"
		"	numeric string=%s\n"
		"	rat=%s\n",
		ix, 
		operator[ix].anetmode,
		operator[ix].alongoperator,
		operator[ix].ashortoperator,
		operator[ix].anumericoperator,
		operator[ix].arat);
	}

	if (register_mode == HI_3G_CARD_COPS_MODE_MANUAL) {
		if (register_format == 
				HI_3G_CARD_COPS_OP_FORMAT_LONG_STRING)
			printf("please input operator name(long string) "
				"you want to register:\n");
		else if (register_format == 
				HI_3G_CARD_COPS_OP_FORMAT_SHORT_STRING)
			printf("please input operator name(short string) "
				"you want to register:\n");
		else
			printf("please input operator name(numeric string) "
				"you want to register:\n");
		fflush(stdin);
		if (NULL == fgets(register_op_name, sizeof(register_op_name), stdin))
			return HI_3G_CARD_ERR_INVAL; 
	}

	ret = HI_3G_RegisterOperator(card, 
				     register_mode, 
				     register_format, 
				     register_op_name);
	if (ret != 0) {
		DBG_OUT("register operator(register mode: (%d)) failed(%d)!\n", 
			register_mode, ret);
		return ret;
	}
	currentoperator = 
		(HI_3G_CARD_OPERATOR_S *)malloc(sizeof(HI_3G_CARD_OPERATOR_S));
	if (currentoperator == NULL) {
		perror("malloc memory for currentoperator failed!\n");
		return ret;
	}
	/* try several times, make sure get currrent operator ok */
	ix = 0;
	int sleep_time = 1;
	do {
		ret = HI_3G_GetCurrentOperator(card, currentoperator);
		if (ret == 0 || ret == HI_3G_CARD_ERR_UNSUPPORT)
			break;
		sleep(sleep_time++);
	} while (++ix < MAX_CMD_RETRIES);
	if (ix >= MAX_CMD_RETRIES || ret == HI_3G_CARD_ERR_UNSUPPORT) {
		printf("get card operator failed(%d)!\n", ret);
		return ret;
	}

	printf("current operator:\n"
		"	netmode :%s\n"
		"	operformat: %s\n"
		"	operator: %s\n"
		"	rat: %s\n",
		currentoperator->anetmode,
		currentoperator->aformatoperator,
		currentoperator->aoperatorinfo,
		currentoperator->arat);

	memset(operatororg, 0, sizeof(HI_3G_OPERATOR_S) * max_operator_num);
	memset(currentoperator, 0, sizeof(HI_3G_CARD_OPERATOR_S));

	return 0;
}

#ifndef __BUSYBOX__
#define ppp_dial_main  main
#endif

int ppp_dial_main(int argc, char *argv[])
{
	int ret;
	int card_num = 0;
	int operator_num = 0;
	int max_card_num = MAX_CARD_NUM;
	char account[8] = {0};
	char password[8] = {0};
	char telephone[8] = "*99#";
	char apn[8] = "3gnet";
	char *argoption[] = {NULL};
	char *cmd;
	int argnum = 0;
	int ix = 0;
	int issr;
	int ber;
	int support_cmd = CMD_SUPPORT_FULL;

	HI_3G_CARD_MODE_E search_mode;
	HI_3G_CARD_ACQORDER_E acqorder;
	HI_3G_CARD_COPS_MODE_E register_mode;
	HI_3G_CARD_COPS_OP_FORMAT_E register_format = 
					HI_3G_CARD_COPS_OP_FORMAT_LONG_STRING;
	HI_3G_CARD_STATE_E status = HI_3G_CARD_STATUS_UNAVAILABLE;

	printf("Usage: %s [telnum] [username] [password] [apn]\n", argv[0]);
	
	switch (argc) {
	case 2:
		snprintf(telephone, sizeof(telephone), "%s", argv[1]);
		snprintf(account, sizeof(account), "");
		snprintf(password, sizeof(password), "");
		snprintf(apn, sizeof(apn), "");
		break;

	case 3:
		snprintf(telephone, sizeof(telephone), "%s", argv[1]);
		snprintf(account, sizeof(account), "%s", argv[2]);
		snprintf(password, sizeof(password), "");
		snprintf(apn, sizeof(apn), "");
		break;

	case 4:
		snprintf(telephone, sizeof(telephone), "%s", argv[1]);
		snprintf(account, sizeof(account), "%s", argv[2]);
		snprintf(password, sizeof(password), "%s", argv[3]);
		snprintf(apn, sizeof(apn), "");
		break;

	case 5:
		snprintf(telephone, sizeof(telephone), "%s", argv[1]);
		snprintf(account, sizeof(account), "%s", argv[2]);
		snprintf(password, sizeof(password), "%s", argv[3]);
		snprintf(apn, sizeof(apn), "%s", argv[4]);
		break;
	default:
		break;
	}

	if (strncmp(telephone, "#777", sizeof("#777")) == 0
	    && (strncasecmp(account, "card", sizeof("card")) == 0)
	    && (strncasecmp(password, "card", sizeof("card")) == 0)) {
		support_cmd = CMD_SUPPORT_LITE;
	}

	printf("[%s],[%s],[%s],[%s]\n", telephone, account, password, apn);
/************************** test1: scan cards   *******************************/
	printf("Test 1: Scan cards\n");

	card = (HI_3G_CARD_S *)malloc(sizeof(HI_3G_CARD_S) * max_card_num);
	if (card == NULL) {
		perror("malloc memory for cards failed!\n");
		return -1;
	}
	ret = HI_3G_ScanCard(card, max_card_num, &card_num);
	if (ret != 0) {
		printf("scan card failed(ret=%d)\n", ret);
		free(card);
		card = NULL;
		return -1;
	}


/************************** test2: init card   ********************************/

	printf("Test 2: Init card\n");

	/* for HUAWEI E303s, should try several times, make sure init card ok*/
	do {
		ret = HI_3G_InitCard(card);
		if (ret == 0)
			break;
		sleep(1);
	} while (++ix < MAX_CMD_RETRIES);
	if (ix >= MAX_CMD_RETRIES) {
		printf("init card failed(ret=%d)\n", ret);
		free(card);
		card = NULL;
		return -1;
	}

/************************** test3: get card status ****************************/

	printf("Test 3: Get card status\n");

	ret = HI_3G_GetCardStatus(card, &status);
	if (ret != 0) {
		printf("get card status failed(ret=%d)\n", ret);
		free(card);
		card = NULL;
		return -1;
	}
	if (status != HI_3G_CARD_STATUS_DISCONNECTED) {
		printf("card status is not ready(status=%d), should be %d\n", 
		       status, HI_3G_CARD_STATUS_DISCONNECTED);
		free(card);
		card = NULL;
		return -1;
	}


/************************** test4: register card ******************************/

	printf("Test 4: Register card\n");
	/* Test 4 only supports WCDMA CARDS, CDMA2000 not supports. */
	if (support_cmd == CMD_SUPPORT_LITE) {
		printf("CDMA cards do not need Test 4: Register card, ignore.\n");
		goto TEST4_EXIT;
	}
	printf("Test 4_0:\n"
		"    search mode: auto;\n"
		"    acqorder: auto;\n"
		"    register mode: auto\n");
	operator = (HI_3G_OPERATOR_S *)malloc(sizeof(HI_3G_OPERATOR_S) * 
					      max_operator_num);
	if (operator == NULL) {
		perror("malloc memory for operators failed!\n");
		free(card);
		card = NULL;
		return -1;
	}
	operatororg = operator;

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_AUTO, 
				    HI_3G_CARD_COPS_MODE_AUTO, 
				    register_format);
	if (ret != 0) {
		printf("Test 4_0 failed(ret=%d)!\n", ret);
	} else
		printf("Test 4_0 Pass!\n");

	printf("Test 4_1:\n"
		"    search mode: auto;\n"
		"    acqorder: auto;\n"
		"    register mode: manul\n"
		"    operator format: long string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_AUTO, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_LONG_STRING);
	if (ret != 0)
		printf("Test 4_1 failed(ret=%d), card may not support op format "
		       "(long string) . Try short/numeric string op format", ret);
	else
		printf("Test 4_1 Pass!\n");

	printf("Test 4_2:\n"
		"    search mode: auto;\n"
		"    acqorder: auto;\n"
		"    register mode: manul\n"
		"    operator format: short string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_AUTO, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_SHORT_STRING);
	if (ret != 0)
		printf("Test 4_2 failed(ret=%d), card may not support op format "
		       "(short string) . Try numeric string op format", ret);
	else
		printf("Test 4_2 Pass!\n");

	printf("Test 4_3:\n"
		"    search mode: auto;\n"
		"    acqorder: auto;\n"
		"    register mode: manul\n"
		"    operator format: numeric string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_AUTO, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_NUMERIC_NUMBER);
	if (ret != 0)
		printf("Test 4_3 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_3 Pass!\n");

	printf("Test 4_4:\n"
		"    search mode: GSM ONLY;\n"
		"    acqorder: GSM prior to WCDMA;\n"
		"    register mode: auto\n");

	ret = set_register_operator(HI_3G_CARD_MODE_GSM_ONLY, 
				    HI_3G_CARD_ACQORDER_GSM_PRIOR_UTRAN, 
				    HI_3G_CARD_COPS_MODE_AUTO, 
				    register_format);
	if (ret != 0)
		printf("Test 4_4 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_4 Pass!\n");

	printf("Test 4_5:\n"
		"    search mode: GSM ONLY;\n"
		"    acqorder: GSM prior to WCDMA;\n"
		"    register mode: manul\n"
		"    operator format: numeric string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_GSM_ONLY, 
				    HI_3G_CARD_ACQORDER_GSM_PRIOR_UTRAN, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_NUMERIC_NUMBER);
	if (ret != 0)
		printf("Test 4_5 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_5 Pass!\n");

	printf("Test 4_6:\n"
		"    search mode: WCDMA ONLY;\n"
		"    acqorder: WCDMA prior to GSM;\n"
		"    register mode: auto\n");

	ret = set_register_operator(HI_3G_CARD_MODE_UTRAN_ONLY, 
				    HI_3G_CARD_ACQORDER_UTRAN_PRIOR_GSM, 
				    HI_3G_CARD_COPS_MODE_AUTO, 
				    register_format);
	if (ret != 0)
		printf("Test 4_6 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_6 Pass!\n");

	printf("Test 4_7:\n"
		"    search mode: WCDMA ONLY;\n"
		"    acqorder: WCDMA prior to GSM;\n"
		"    register mode: manul\n"
		"    operator format: numeric string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_UTRAN_ONLY, 
				    HI_3G_CARD_ACQORDER_UTRAN_PRIOR_GSM, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_NUMERIC_NUMBER);
	if (ret != 0)
		printf("Test 4_7 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_7 Pass!\n");

	printf("Test 4_8:\n"
		"    search mode: auto;\n"
		"    acqorder: GSM prior to WCDMA;\n"
		"    register mode: auto\n");

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_GSM_PRIOR_UTRAN, 
				    HI_3G_CARD_COPS_MODE_AUTO, 
				    register_format);
	if (ret != 0)
		printf("Test 4_8 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_8 Pass!\n");

	printf("Test 4_9:\n"
		"    search mode: auto;\n"
		"    acqorder: GSM prior to WCDMA;\n"
		"    register mode: manul\n"
		"    operator format: numeric string\n");

	ret = set_register_operator(HI_3G_CARD_MODE_AUTO, 
				    HI_3G_CARD_ACQORDER_GSM_PRIOR_UTRAN, 
				    HI_3G_CARD_COPS_MODE_MANUAL, 
				    HI_3G_CARD_COPS_OP_FORMAT_NUMERIC_NUMBER);
	if (ret != 0)
		printf("Test 4_9 failed(ret=%d)!\n", ret);
	else
		printf("Test 4_9 Pass!\n");

TEST4_EXIT:
	if (operatororg)
		free(operatororg);
	operatororg = NULL;
	if (currentoperator)
		free(currentoperator);
	currentoperator = NULL;

/************************** Test 5: Set PDP ******************************/
	printf("Test 5: Set PDP\n");
	/* Test 5 only supports WCDMA CARDS, CDMA2000 not supports. */
	if (support_cmd == CMD_SUPPORT_LITE) {
		printf("CDMA cards do not need Test 5: Set PDP, ignore.\n");
		goto TEST5_EXIT;
	}
	ix = 0;
	int sleep_time = 1;
	currentoperator = 
		(HI_3G_CARD_OPERATOR_S *)malloc(sizeof(HI_3G_CARD_OPERATOR_S));
	if (currentoperator == NULL) {
		perror("malloc memory for currentoperator failed!\n");
		free(card);
		card = NULL;
		return ret;
	}

	do {
		ret = HI_3G_GetCurrentOperator(card, currentoperator);
		if (ret == 0 || ret == HI_3G_CARD_ERR_UNSUPPORT)
			break;
		sleep(sleep_time++);
	} while (++ix < MAX_CMD_RETRIES);
	if (ix >= MAX_CMD_RETRIES) {
		printf("get card operator failed(%d)!\n", ret);
		free(card);
		card = NULL;
		free(currentoperator);
		currentoperator = NULL;
		return ret;
	}

	if (strncmp(currentoperator->aoperatorinfo, "CHN-CUGSM", strlen("CHN-CUGSM")) == 0
	|| (strncmp(currentoperator->aoperatorinfo, "CU-GSM", strlen("CU-GSM")) == 0) 
	|| (strncmp(currentoperator->aoperatorinfo, "46001", strlen("46001")) == 0)) {
		snprintf(apn, sizeof(apn), "3gnet");
		snprintf(telephone, sizeof(telephone), "*99#");
	} else if (strncmp(currentoperator->aoperatorinfo, "CHINA  MOBILE", strlen("CHINA  MOBILE")) == 0
	|| (strncmp(currentoperator->aoperatorinfo, "CMCC", strlen("CMCC")) == 0) 
	|| (strncmp(currentoperator->aoperatorinfo, "46000", strlen("46000")) == 0)) {
		snprintf(apn, sizeof(apn), "cmnet");
		snprintf(telephone, sizeof(telephone), "*98*1#");
	} else {
		printf("not recognize operator!\n");
		free(card);
		card = NULL;
		free(currentoperator);
		currentoperator = NULL;
		return -1;
	}
	ret = HI_3G_SetApn(card, apn);
	if (ret != 0) {
		printf("set apn failed(ret=%d)!\n", ret);
		free(card);
		card = NULL;
		free(currentoperator);
		currentoperator = NULL;
		return -1;
	}

	HI_3G_PDP_S *pdp = (HI_3G_PDP_S *)malloc(sizeof(HI_3G_PDP_S));
	if (pdp == NULL) {
		printf("malloc pdp failed!\n");
		free(card);
		card = NULL;
		free(currentoperator);
		currentoperator = NULL;
		return -1;
	}
	ret = HI_3G_GetApn(card, pdp);
	if (ret != 0) {
		printf("get pdp failed(ret=%d)!\n", ret);
		free(card);
		card = NULL;
		free(currentoperator);
		currentoperator = NULL;
		free(pdp);
		pdp = NULL;
		return -1;
	} else
		printf("get current pdp successfully:\n"
			 "	cid:             %s\n"
			 "	pdp_type:        %s\n"
			 "	apn:             %s\n"
			 "	pdp_ipaddr:      %s\n"
			 "	d_comp:          %s\n"
			 "	h_comp:          %s\n",
			 pdp->acid, 
			 pdp->apdptype, 
			 pdp->aapn, 
			 pdp->apdpipaddr, 
			 pdp->adcomp, 
			 pdp->ahcomp);

	free(pdp);
	pdp = NULL;
TEST5_EXIT:
	if (currentoperator)
		free(currentoperator);
	currentoperator = NULL;
	
/**************************Test 6: Get card quality******************************/
	printf("Test 6: Get card quality\n");

	ret = HI_3G_GetQuality(card, &issr, &ber);
	if (ret != 0)
		printf("get card quality failed(ret=%d)!\n", ret);
	else {
		printf("get card quality successfully\n"
			"	issr: %d,"
			"	ber: %d\n",
			issr, ber);
	}

/************************** Test 7: Connect card ******************************/
	/* try several times, make sure to connect ok */
	ix = 0;
	do {
		ret = HI_3G_ConnectCard(card, account, password, 
			      telephone, 0, argoption);
		if (ret == 0)
		printf("Test 7: Connect card Pass!\n");
		break;
		sleep(1);
	} while (++ix < MAX_CMD_RETRIES);
	if (ix >= MAX_CMD_RETRIES) {
		printf("connect card failed(ret=%d)!\n", ret);
		free(card);
		card = NULL;
		return -1;
	}


/*
	printf("***************************************************\n");
	printf("Test 8: Get card dataflow\n");
	printf("***************************************************\n\n");
	/* Test 8 only supports WCDMA CARDS, CDMA2000 not supports. */
/*	if (support_cmd == CMD_SUPPORT_LITE) {
		printf("CDMA cards no need Test 8: Get card dataflow, ignore.\n");
		goto TEST8_EXIT;
	}
	dataflow = (HI_3G_CARD_FLOW_S *)malloc(sizeof(HI_3G_CARD_FLOW_S));
	if (dataflow == NULL) {
		perror("malloc memory for dataflow failed!\n");
		free(card);
		card = NULL;
		return -1;
	}
	ret = HI_3G_GetDataFlow(card, dataflow);
	if (ret != 0)
		printf("get card dataflow failed(ret=%d)!\n", ret);
	else {
		printf("get card dataflow successfully:\n"
			"	lastdstime: %x\n"
			"	totaldstime: %x\n"
			"	lasttxflow: %llx\n"
			"	totaltxflow: %llx\n"
			"	lastrxflow: %llx\n"
			"	totalrxflow: %llx\n",
			dataflow->lastdstime,
			dataflow->totaldstime,
			dataflow->lasttxflow,
			dataflow->totaltxflow,
			dataflow->lastrxflow,
			dataflow->totalrxflow);
		printf("***************************************************\n");
		printf("Test 8: Get card dataflow Pass!\n");
		printf("*************************************************\n\n");
	}

	free(dataflow);
	dataflow = NULL;

TEST8_EXIT:
*/
/*	printf("***************************************************\n");
*	printf("Test 9: Disconnect card\n");
*	printf("***************************************************\n\n");
*
*	ret = HI_3G_DisConnectCard(card);
*	if (ret != 0) {
*		printf("disconnect card failed(ret=%d)!\n", ret);
*		free(card);
*		card = NULL;
*		return -1;
*	}
*	printf("***************************************************\n");
*	printf("Test 9: Disconnect card Pass!\n");
*	printf("***************************************************\n\n");
*/
/*			printf("***************************************************\n");
			printf("Test 10: reconnect card\n");
			printf("***************************************************\n\n");
			/* try several times, make sure to connect ok */
/*			ix = 0;
			do {
				ret = HI_3G_ConnectCard(card, account, password, 
					      telephone, 0, argoption);
				if (ret == 0)
				printf("***************************************************\n");
				printf("Test 10: reconnect card Pass!\n");
				printf("***************************************************\n\n");
				break;
				sleep(1);
			} while (++ix < MAX_CMD_RETRIES);
				if (ix >= MAX_CMD_RETRIES) {
					printf("connect card failed(ret=%d)!\n", ret);
					free(card);
					card = NULL;
//					return -1;
//					}
			printf("***************************************************\n");
			printf("Test 11: DeInit card\n");
			printf("***************************************************\n\n");

			ret = HI_3G_DeInitCard(card);
			if (ret != 0) {
				printf("deinit card failed(ret=%d)!\n", ret);
				free(card);
				card = NULL;
				return -1;
			}
			printf("***************************************************\n");
			printf("Test 11: DeInit card Pass!\n");
			printf("***************************************************\n\n");

			printf("***************************************************\n");
			printf("Test 12: ReInit card\n");
			printf("***************************************************\n\n");
*/			/* for HUAWEI E303s, should try several times, make sure init card ok*/
/*			do {
				ret = HI_3G_InitCard(card);
				if (ret == 0)
					break;
					sleep(1);
				} while (++ix < MAX_CMD_RETRIES);
				if (ix >= MAX_CMD_RETRIES) {
					printf("init card failed(ret=%d)\n", ret);
					free(card);
					card = NULL;
					return -1;
			}

			printf("***************************************************\n");
			printf("Test 12: ReInit card Pass!\n");
			printf("***************************************************\n\n");

			printf("***************************************************\n");
			printf("Test 13: ReConnect card\n");
			printf("***************************************************\n\n");
*/
			/* try several times, make sure to connect ok */
/*			ix = 0;
			do {
				ret = HI_3G_ConnectCard(card, account, password, 
					      telephone, 0, argoption);
				if (ret == 0)
					break;
					sleep(1);
				} while (++ix < MAX_CMD_RETRIES);
				if (ix >= MAX_CMD_RETRIES) {
					printf("connect card failed(ret=%d)!\n", ret);
					free(card);
					card = NULL;
					return -1;
			}

			printf("***************************************************\n");
			printf("Test 13: ReConnect card Pass!\n");
			printf("***************************************************\n\n");
				
			free(card);
			card = NULL;
			return 0;
			}
*/
}
