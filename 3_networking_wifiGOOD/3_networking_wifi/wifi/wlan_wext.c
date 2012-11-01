/** @file  wlan_wext.c 
 * @brief This file contains ioctl functions
 *
 *  Copyright �Marvell International Ltd. and/or its affiliates, 2003-2007
 */
/********************************************************
 Change log:
 10/10/05: Add Doxygen format comments
 12/23/05: Modify FindBSSIDInList to search entire table for
 	 	 	 duplicate BSSIDs when earlier matches are not compatible
 12/26/05: Remove errant memcpy in wlanidle_off; overwriting stack space
 01/05/06: Add kernel 2.6.x support
 01/11/06: Conditionalize new scan/join functions.
 	 	 	 Update statics/externs.  Move forward decl. from wlan_decl.h
 04/06/06: Add TSPEC, queue metrics, and MSDU expiry support
 04/10/06: Add hostcmd generic API
 04/18/06: Remove old Subscrive Event and add new Subscribe Event
 	 	 	 implementation through generic hostcmd API
 05/04/06: Add IBSS coalescing related new iwpriv command
 08/29/06: Add ledgpio private command
 10/23/06: Validate setbcnavg/setdataavg command parameters and
 return error if out of range
 ********************************************************/

#include "include.h"
#include "wlan_dev.h"
#include "wlan_debug.h"
#include "wlan_wext.h"

extern struct rt_wlan_dev wlan_eth;
extern int wlan_set_scan(struct rt_wlan_dev * dev, char *extra);
unsigned char mypsk[32];
extern int wlan_set_wap(WlanCard *cardinfo, unsigned char * dstbssid);
extern int wlan_set_multicast_list(WlanCard *cardinfo);
extern int wlan_cmd_802_11_set_wep(WlanCard *cardinfo, u32 cmd_oid);
extern int wlan_Control_Mac(WlanCard *cardinfo, u16 action);

int Wlan_Association(WlanConfig* config, WlanInfo * wlaninfo)
{
	WlanInfo * Wlan = wlaninfo;
	WlanCard *card = Wlan->card;
	wlan_set_wap(card, config->MacAddr);
	return 0;
}

int wlan_set_mac_multicast_add(WlanCard *cardinfo,
		Multi_Addr_Struct *mac_address_arry)
{
	WlanCard *card = cardinfo;
	int i = 0;

	if ((mac_address_arry == NULL) || (cardinfo == NULL))
		return -1;

	if (mac_address_arry->addressnum > MRVDRV_MAX_MULTICAST_LIST_SIZE)
		return -1;
	while (i < mac_address_arry->addressnum)
	{
		rt_memcpy(card->MulticastList[i],
				(u8*) (&mac_address_arry->multi_addr[i][0]),
				MRVDRV_ETH_ADDR_LEN);
		i++;
	}
	card->NumOfMulticastMACAddr = mac_address_arry->addressnum;

	wlan_set_multicast_list(card);
	return 0;
}

int set_wep_materials(WlanCard *cardinfo, Wep_Key_Set_ArrayPtr key_set_arg)
{
	int ret = WLAN_STATUS_SUCCESS;
	WlanCard *card = cardinfo;

	MRVL_WEP_KEY *pWep;
	int index, i = 0;

	if (key_set_arg->defaut_key_index >= MRVL_NUM_WEP_KEY)
	{
		return -12;
	}
	else
	{
		index = key_set_arg->defaut_key_index;
	}

	if (key_set_arg->KeyLength[index] == 0 || key_set_arg->KeyLength[index]
			> MAX_WEP_KEY_SIZE)
		return -13;

	card->CurrentWepKeyIndex = index;
	for (i = 0; i < MRVL_NUM_WEP_KEY; i++)
	{
		pWep = &card->WepKey[i];
		if ((key_set_arg->KeyLength[i] != 0) && (key_set_arg->KeyLength[i]
				<= MAX_WEP_KEY_SIZE))
		{
			rt_memcpy(pWep->KeyMaterial, &key_set_arg->Key_value[i][0],
					key_set_arg->KeyLength[i]);
			if (key_set_arg->KeyLength[i] > MIN_WEP_KEY_SIZE)
			{
				pWep->KeyLength = MAX_WEP_KEY_SIZE;
			}
			else
			{
				if (key_set_arg->KeyLength[i] > 0)
				{
					pWep->KeyLength = MIN_WEP_KEY_SIZE;
				}
			}
		}
		else
		{
			pWep->KeyLength = 0;
			rt_memset(pWep->KeyMaterial, 0x00, MAX_WEP_KEY_SIZE);
		}
	}
	card->SecInfo.WEPStatus = Wlan802_11WEPEnabled;
	ret = wlan_cmd_802_11_set_wep(card, OID_802_11_ADD_WEP);
	if (ret)
	{
		WlanDebug(WlanErr,"cmd:HostCmd_CMD_802_11_SET_WEP failed\r\n");
		return ret;
	}

	WlanDebug(WlanEncy,"Length=%d CurrentWepKeyIndex=%d\n",
			key_set_arg->KeyLength[card->CurrentWepKeyIndex], card->CurrentWepKeyIndex);
	if (card->SecInfo.WEPStatus == Wlan802_11WEPEnabled)
	{
		card->CurrentPacketFilter |= HostCmd_ACT_MAC_WEP_ENABLE;
	}
	else
	{
		card->CurrentPacketFilter &= ~HostCmd_ACT_MAC_WEP_ENABLE;
	}

	WlanDebug(WlanMsg, "CurrentPacketFilter => 0x%x\n", card->CurrentPacketFilter);
	ret = wlan_Control_Mac(card, card->CurrentPacketFilter);
	if (ret)
	{
		WlanDebug(WlanErr,"cmd:wlan_Control_Mac failed\r\n");
		return ret;
	}

	return WLAN_STATUS_SUCCESS;
}

int set_wep_no_key(WlanCard *cardinfo)
{
	WlanCard *card = cardinfo;
	int ret;
	card->SecInfo.WEPStatus = Wlan802_11WEPDisabled;
	card->CurrentPacketFilter &= ~HostCmd_ACT_MAC_WEP_ENABLE;
	card->SecInfo.EncryptionMode == CIPHER_NONE;

	ret = wlan_Control_Mac(card, card->CurrentPacketFilter);
	if (ret)
	{
		WlanDebug(WlanErr,"cmd:wlan_Control_Mac failed\r\n");
		return ret;
	}
	return ret;
}

int wlan_set_infrastructure(WlanCard *cardinfo)
{
	WlanCard *card = cardinfo;
	int ret = WLAN_STATUS_SUCCESS;

	WlanDebug(WlanErr,"wlan_set_infrastructure\r\n");
	card->InfrastructureMode == Wlan802_11Infrastructure;
	ret = wlan_cmd_802_11_snmp_mib(card, HostCmd_ACT_SET,
			OID_802_11_INFRASTRUCTURE_MODE, NULL);
	return ret;
}

/** 
 *  @brief Set/Get WPA IE   
 *
 *  @param priv         A pointer to wlan_private structure
 *  @param ie_data_ptr  A pointer to IE
 *  @param ie_len       Length of the IE
 *
 *  @return             WLAN_STATUS_SUCCESS --success, otherwise fail
 */
static int wlan_set_wpa_ie_helper(WlanCard *cardinfo, u8 * ie_data_ptr,
		u16 ie_len)
{
	WlanCard *card = cardinfo;
	int ret = WLAN_STATUS_SUCCESS;

	if (ie_len)
	{
		rt_memset(card->Wpa_ie, 0, sizeof(card->Wpa_ie));
		if (ie_len > sizeof(card->Wpa_ie))
		{
			WlanDebug(WlanErr,"failed to copy WPA IE, too big \n");
			return -2;
		}
		rt_memcpy(card->Wpa_ie, ie_data_ptr, ie_len);
		card->Wpa_ie_len = ie_len;
		WlanDebug(WlanEncy,"Set Wpa_ie_len=%d IE=%#x\n",card->Wpa_ie_len, card->Wpa_ie[0]);
		hexdump("Wpa_ie", card->Wpa_ie, card->Wpa_ie_len);

		if (card->Wpa_ie[0] == WPA_IE)
		{
			card->SecInfo.WPAEnabled = TRUE;
			card->SecInfo.WPA2Enabled = FALSE;
		}
		else if (card->Wpa_ie[0] == RSN_IE)
		{
			card->SecInfo.WPA2Enabled = TRUE;
			card->SecInfo.WPAEnabled = FALSE;
		}
		else
		{
			card->SecInfo.WPAEnabled = FALSE;
			card->SecInfo.WPA2Enabled = FALSE;
		}
	}
	else
	{
		rt_memset(card->Wpa_ie, 0, sizeof(card->Wpa_ie));
		card->Wpa_ie_len = ie_len;
		WlanDebug(WlanEncy,"Reset Wpa_ie_len=%d IE=%#x\n",card->Wpa_ie_len, card->Wpa_ie[0]);
		card->SecInfo.WPAEnabled = FALSE;
		card->SecInfo.WPA2Enabled = FALSE;
	}

	return ret;
}

int wlan_wpa_set_key(void *ctx, int alg, const u8 *addr, int key_idx,
		int set_tx, const u8 *seq, u8 seq_len, const u8 *key, u8 key_len)
{
	int ret = WLAN_STATUS_SUCCESS;
	WlanInfo * Wlan = wlan_eth.priv;
	WlanCard *card = Wlan->card;
	static WLAN_802_11_KEY pKey;
	pKey.Length = sizeof(WLAN_802_11_KEY);
	pKey.KeyIndex = key_idx;

	pKey.KeyLength = key_len;
	rt_memcpy((void*) pKey.BSSID, addr, ETH_ALEN);

	rt_memcpy((void*) pKey.KeyMaterial, key, key_len);

	// current driver only supports key length of up to 32 bytes
	if (pKey.KeyLength > MRVL_MAX_WPA_KEY_LENGTH)
	{
		WlanDebug(WlanErr, " Error in key length \n");
		return WLAN_STATUS_FAILURE;
	}

	ret = wlan_cmd_802_11_key_material(card, HostCmd_ACT_SET, KEY_INFO_ENABLED,
			&pKey);
	if (ret)
	{
		return ret;
	}
	return ret;
}

void wlan_set_wpa_info(WlanConfig* config, WlanInfo * wlaninfo)
{
	return;
}

