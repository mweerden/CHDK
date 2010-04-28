#include "camera.h"
#ifdef CAM_CHDK_PTP

#include "platform.h"
#include "stdlib.h"
#include "ptp.h"
#include "script.h"

#define BUF_SIZE 0x20000 // XXX what's a good camera-independent value?

static int handle_ptp(
                int h, ptp_data *data, int opcode, int sess_id, int trans_id,
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

static int recv_ptp_data(ptp_data *data, char *buf, int size)
  // repeated calls per transaction are ok
{
  while ( size >= BUF_SIZE )
  {
    data->recv_data(data->handle,buf,BUF_SIZE,0,0);
    // XXX check for success??

    size -= BUF_SIZE;
    buf += BUF_SIZE;
  }
  if ( size != 0 )
  {
    data->recv_data(data->handle,buf,size,0,0);
    // XXX check for success??
  }

  return 1;
}

static int send_ptp_data(ptp_data *data, char *buf, int size)
  // repeated calls per transaction are *not* ok
{
  int tmpsize;
  
  tmpsize = size;
  while ( size >= BUF_SIZE )
  {
    if ( data->send_data(data->handle,buf,BUF_SIZE,tmpsize,0,0,0) )
    {
      return 0;
    }

    tmpsize = 0;
    size -= BUF_SIZE;
    buf += BUF_SIZE;
  }
  if ( size != 0 )
  {
    if ( data->send_data(data->handle,buf,size,tmpsize,0,0,0) )
    {
      return 0;
    }
  }

  return 1;
}

static int handle_ptp(
               int h, ptp_data *data, int opcode, int sess_id, int trans_id,
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

    case PTP_CHDK_Version:
      ptp.num_param = 2;
      ptp.param1 = PTP_CHDK_VERSION_MAJOR;
      ptp.param2 = PTP_CHDK_VERSION_MINOR;
      break;

    case PTP_CHDK_GetMemory:
      if ( param2 == 0 || param3 < 1 ) // null pointer or invalid size?
      {
        ptp.code = PTP_RC_GeneralError;
        break;
      }

      if ( !send_ptp_data(data,(char *) param2,param3) )
      {
        ptp.code = PTP_RC_GeneralError;
      }
      break;
      
    case PTP_CHDK_SetMemory:
      if ( param2 == 0 || param3 < 1 ) // null pointer or invalid size?
      {
        ptp.code = PTP_RC_GeneralError;
        break;
      }

      data->get_data_size(data->handle); // XXX required call before receiving
      if ( !recv_ptp_data(data,(char *) param2,param3) )
      {
        ptp.code = PTP_RC_GeneralError;
      }
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
        if ( !recv_ptp_data(data,(char *) buf,s) )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        ptp.num_param = 1;
        ptp.param1 = ((int (*)(int,int,int,int,int,int,int,int,int,int)) buf[0])(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10]);

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

      if ( !recv_ptp_data(data,temp_data,temp_data_size) )
      {
        ptp.code = PTP_RC_GeneralError;
      }
      break;

    case PTP_CHDK_UploadFile:
      {
        FILE *f;
        int s,r,fn_len;
        char *buf, *fn;

        s = data->get_data_size(data->handle);

        recv_ptp_data(data,(char *) &fn_len,4);
        s -= 4;

        fn = (char *) malloc(fn_len+1);
        if ( fn == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }
        fn[fn_len] = '\0';

        recv_ptp_data(data,fn,fn_len);
        s -= fn_len;

        f = fopen(fn,"wb");
        if ( f == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          free(fn);
          break;
        }
        free(fn);

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
            recv_ptp_data(data,buf,BUF_SIZE);
            fwrite(buf,1,BUF_SIZE,f);
            s -= BUF_SIZE;
          } else {
            recv_ptp_data(data,buf,s);
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
        int tmp,t,s,r,fn_len;
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

        buf = (char *) malloc(BUF_SIZE);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        tmp = s;
        t = s;
        while ( (r = fread(buf,1,(t<BUF_SIZE)?t:BUF_SIZE,f)) > 0 )
        {
          t -= r;
          // cannot use send_ptp_data here
          data->send_data(data->handle,buf,r,tmp,0,0,0);
          tmp = 0;
        }
        fclose(f);
        // XXX check that we actually read/send s bytes! (t == 0)

        ptp.num_param = 1;
        ptp.param1 = s;

        free(buf);

        break;
      }
      break;

    case PTP_CHDK_ExecuteScript:
      {
        int s;
        char *buf;

        if ( param2 != PTP_CHDK_SL_LUA )
        {
          ptp.code = PTP_RC_ParameterNotSupported;
          break;
        }

        s = data->get_data_size(data->handle);
        
        buf = (char *) malloc(s);
        if ( buf == NULL )
        {
          ptp.code = PTP_RC_GeneralError;
          break;
        }

        recv_ptp_data(data,buf,s);

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

#endif // CAM_CHDK_PTP
