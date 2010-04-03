#include "platform.h"
#include "stdlib.h"
#include "ptp.h"
#include "script.h"

int handle_ptp(int h, ptp_data *data, int opcode, int sess_id, int trans_id,
               int param1, int param2, int param3, int param4, int param5);

void init_chdk_ptp()
{
  int r;
 
  // wait until ptp_handlers_info is initialised and add CHDK PTP interface
  r = 0x17;
  while ( r==0x17 )
  {
    r = add_ptp_handler(PTP_OC_CHDK,handle_ptp,0);
    msleep(250);
  }

  ExitTask();
}

int handle_ptp(int h, ptp_data *data, int opcode, int sess_id, int trans_id,
               int param1, int param2, int param3, int param4, int param5)
{
  static char *temp_data = NULL;
  static int temp_data_size = 0;
  PTPContainer ptp;

  // initialise default response
  memset(&ptp,0,sizeof(PTPContainer));
  ptp.code = PTP_RC_OK;
  ptp.sess_id = sess_id;
  ptp.trans_id = trans_id;
  ptp.num_param = 0;

  // handle command
  switch ( param1 )
  {

    case PTP_CHDK_Shutdown:
      ptp.code = PTP_RC_GeneralError; // default in case shutdown/reboot fails
      switch ( param2 )
      {
        case 0: // hard shutdown
          shutdown();
          break;
        case 1: // soft shutdown
          shutdown_soft();
          ptp.code = PTP_RC_OK; // shutdown is not immediate
          break;
        case 2: // reboot
          reboot(NULL);
          break;
        case 3: // reboot using firmware update
          {
            FILE *f;
            int s = data->get_data_size(data->handle);
            char *fn = (char *) malloc(s+1);

            sprintf(fn,"%i",s);
            script_console_add_line(fn);
            if ( fn == NULL )
            {
              ptp.code = PTP_RC_GeneralError;
              break;
            }
            fn[s] = '\0';

            data->recv_data(data->handle,fn,s,0,0);
            
            if ( s == 0 )
            {
              free(fn);
              fn = "A/PS.FI2";
            }

            if ( (f = fopen(fn,"rb")) == NULL )
            {
              ptp.code = PTP_RC_GeneralError;
            } else {
              fclose(f);
              reboot(fn);
            }
           
            if ( s != 0 )
            {
              free(fn);
            }
            break;
          }
        default:
          ptp.code = PTP_RC_ParameterNotSupported;
          break;
      }
      break;

    case PTP_CHDK_GetMemory:
      {
        if ( param2 == 0 || param3 < 1 ) // null pointer or invalid size?
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        if ( data->send_data(data->handle,(char *) param2,param3,param3,0,0,0) )
        {
          ptp.code = PTP_RC_GeneralError;
        }
        break;
      }
      
    case PTP_CHDK_SetMemoryLong:
      *((volatile long *) param2) = param3;
      break;

    case PTP_CHDK_CallFunction:
      {
        int s;
        int *buf = (int *) malloc((10+1)*sizeof(int));

        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        s = data->get_data_size(data->handle);
        data->recv_data(data->handle,(char *) buf,s,0,0);

        ptp.num_param = 1;
        ptp.param1 = ((int (*)(int,int,int,int,int,int,int,int,int,int)) buf[0])(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10]);

        free(buf);
        break;
      }

    case PTP_CHDK_GetPropCase:
      {
        int *buf, i;

        if ( param3 < 1 ) // invalid number of properties
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        buf = (int *) malloc(param3*sizeof(int));
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        for (i=0; i<param3; i++)
        {
          get_property_case(param2+i,(char *) &buf[i],4);
        }

        if ( data->send_data(data->handle,(char *) buf,param3*sizeof(int),param3*sizeof(int),0,0,0) )
        {
          ptp.code = PTP_RC_GeneralError;
        }
        free(buf);
        break;
      }

    case PTP_CHDK_GetParamData:
      {
        extern long* FlashParamsTable[];
        char *buf, *p;
        int i, s;

        if ( param3 < 1 ) // invalid number of parameters
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        // calculate required size for all properties (and lengths)
        s = 0;
        for (i=0; i<param3; i++)
        {
          s += 4 + (FlashParamsTable[param2+i][1]>>16); // room for size and actual data
        }

        buf = (char *) malloc(s);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        // construct result
        p = buf;
        for (i=0; i<param3; i++)
        {
          int t = FlashParamsTable[param2+i][1]>>16;
          // write size
          memcpy(p,&t,4);
          p += 4;
          // write value
          get_parameter_data(param2+i,p,t);
          p += t;
        }

        if ( data->send_data(data->handle,buf,s,s,0,0,0) )
        {
          ptp.code = PTP_RC_GeneralError;
        }
        free(buf);
        break;
      }
      
    case PTP_CHDK_TempData:
      if ( temp_data != NULL )
      {
        free(temp_data);
        temp_data = NULL;
      }

      temp_data_size = data->get_data_size(data->handle);

      temp_data = (char *) malloc(temp_data_size);
      if ( temp_data == NULL )
      {
        ptp.code = PTP_RC_GeneralError;
        break;
      }

      data->recv_data(data->handle,temp_data,temp_data_size,0,0);

      break;

    case PTP_CHDK_UploadFile:
      {
        FILE *f;
        int s,r,fn_len;
        char *buf, *fn;

        s = data->get_data_size(data->handle);

        data->recv_data(data->handle,(char *) &fn_len,4,0,0);
        s -= 4;

        fn = (char *) malloc(fn_len+1);
        if ( fn == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }
        fn[fn_len] = '\0';

        data->recv_data(data->handle,fn,fn_len,0,0);
        s -= fn_len;

        f = fopen(fn,"wb");
        if ( f == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          free(fn);
          break;
        }
        free(fn);

#define BUF_SIZE 4096
        buf = (char *) malloc(BUF_SIZE);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }
        while ( s > 0 )
        {
          if ( s >= BUF_SIZE )
          {
            r = data->recv_data(data->handle,buf,BUF_SIZE,0,0);
            fwrite(buf,1,BUF_SIZE,f);
            s -= BUF_SIZE;
          } else {
            data->recv_data(data->handle,buf,s,0,0);
            fwrite(buf,1,s,f);
            s = 0;
          }
        }

        fclose(f);

        free(buf);
        break;
      }
      
    case PTP_CHDK_DownloadFile:
      {
        FILE *f;
        int s,r,fn_len;
        char *buf, *fn;

        if ( temp_data == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        fn = (char *) malloc(temp_data_size+1);
        if ( fn == NULL )
        {
          free(temp_data);
          temp_data = NULL;
          ptp.code = PTP_RC_GeneralError;
          break;
        }
        memcpy(fn,temp_data,temp_data_size);
        fn[temp_data_size] = '\0';

        free(temp_data);
        temp_data = NULL;

        f = fopen(fn,"rb");
        if ( f == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          free(fn);
          break;
        }
        free(fn);

        fseek(f,0,SEEK_END);
        s = ftell(f);
        fseek(f,0,SEEK_SET);

        buf = (char *) malloc(s);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          fclose(f);
          break;
        }

        fread(buf,1,s,f);
        fclose(f);

        data->send_data(data->handle,buf,s,s,0,0,0);
        ptp.num_param = 1;
        ptp.param1 = s;

        free(buf);

        break;
      }
      break;

    case PTP_CHDK_SwitchMode:
      if ( param2 != 0 && param2 != 1 )
      {
        ptp.code = PTP_RC_ParameterNotSupported;
      } else {
        switch_mode(param2);
      }
      break;

    case PTP_CHDK_ExecuteLUA:
      {
        int s;
        char *buf;

        s = data->get_data_size(data->handle);
        
        buf = (char *) malloc(s);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        data->recv_data(data->handle,buf,s,0,0);

        lua_script_exec(buf);

        free(buf);

        break;
      }

    default:
      ptp.code = PTP_RC_ParameterNotSupported;
      break;
  }

  // send response
  data->send_resp( data->handle, &ptp );
  
  return 1;
}
