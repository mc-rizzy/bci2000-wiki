////////////////////////////////////////////////////////////////////////////////
// Authors: lijintao@SH-lijintao
// Description: NeuSenWADC header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_NEUSENWADC_H
#define INCLUDED_NEUSENWADC_H

#include "BufferedADC.h"
#include "NeuracleSDK.h"
#include <Windows.h>

class NeuSenWADC : public BufferedADC
{
 public:
  NeuSenWADC();
  ~NeuSenWADC();
  void OnPublish() override;
  void OnAutoConfig() override;
  void OnPreflight( SignalProperties& Output ) const override;
  void OnInitialize( const SignalProperties& Output ) override;
  void OnStartAcquisition() override;
  void DoAcquire( GenericSignal& Output ) override;
  void OnStopAcquisition() override;

 private:
	 NeuracleController* m_Controller = nullptr;
	 bool m_trigger_box = false;
	 char* m_device_address = nullptr;
	 char* m_trigger_address = nullptr;

	 int m_sample_rate = 0;
	 int m_sample_block = 1;

	 HANDLE m_com = INVALID_HANDLE_VALUE;
	 LPCSTR m_port_num = nullptr;
};

#endif
