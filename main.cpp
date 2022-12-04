// Imports
#include "lodepng/lodepng.cpp"
#include "lodepng/lodepng.h"
#include "headers.h"

using namespace std;

vector<int> ampdb(0);
vector<int> frequency(0);
vector<unsigned char> logo;

//SFML global declarations for seeking play time
sf::SoundBuffer buffer;
sf::Sound sound(buffer);

pthread_t threads[1]; //for multi threading
int rc1;
int flag=0,temp=0,loading=0,timerFlag=0;
typedef unsigned long long timestamp_t;
int j=0;
float r=23.0;  //circle "r"
float d=0.3;     //cuboid width/2
float deg=15.0;
double SAMPLE_COUNT;
double SAMPLE_RATE;
kiss_fft_cpx in[N], out[N];
int styleselect=0;
int NO_STYLE=5;
int countr=0,countg=0,countb=0;
float curtime;
string fName;

static timestamp_t

get_timestamp ()
{
    struct timeval now;
    gettimeofday (&now, NULL);
    return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

void getFft(const kiss_fft_cpx in[N], kiss_fft_cpx out[N])
{
    kiss_fft_cfg cfg;
    if ((cfg = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
    {
        kiss_fft(cfg, in, out);
        free(cfg);
    }
    else
    {
        printf("not enough memory?\n");
        exit(-1);
    }

}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90,(float)w/(float)h, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

void drawStrokeText(char*str,int x,int y,int z)
{
    glLineWidth(1);
    char *c;
    float wt = glutStrokeLength(GLUT_STROKE_ROMAN,(unsigned char*)str)*0.4;
    glPushMatrix();
    glTranslatef(-(wt/2), y+8,z);
    glScalef(0.4f,0.4f,0.4f);
    glColor3f(1,1,1);
    for (c=str; *c !=0 /*NULL*/; c++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();
}

void instructText(char*str,int x,int y,int z)
{
    char *c;
    glLineWidth(1);
    float wt = glutStrokeLength(GLUT_STROKE_ROMAN,(unsigned char*)str)*0.1;
    glPushMatrix();
    glTranslatef(-wt/2, y+8,z);
    glScalef(0.1f,0.1f,0.1f);

    for (c=str; *c != 0 /*NULL*/ ; c++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();
}

void init()
{
    glEnable(GL_DEPTH_TEST);  //enables DEPTH_TEST
    glDepthFunc(GL_LEQUAL);   //Lesser depth & EQUAL depth valued objects displayed in the front

    //Do anti alias
    glEnable(GL_MULTISAMPLE_3DFX);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_POLYGON_SMOOTH);
}

void processKeys(unsigned char key, int x, int y)
{

    if (key == 27 || key=='q'|| key=='Q') // escape key
        exit(0);
    else if(key == 32) // spacebar key
    {
        if(temp==0)
        {
            flag=1;
            temp++;
            sound.play();
        }
        else if(temp==1)
        {
            sound.pause();
            temp--;
        }
        if(flag==0)
        {
            flag=1;
            temp=1;
            sound.play();
        }
    }
    else if(key == 77 || key==109)
    {
        sound.pause();
        flag=0;
    }
}

void processSpecialKeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_LEFT:
        if(styleselect==0)
        {
            styleselect=NO_STYLE-1;
        }
        else
        {
            styleselect--;
        }
        break;
    case GLUT_KEY_RIGHT:
        styleselect=(styleselect+1)%NO_STYLE;
        break;
    case GLUT_KEY_UP:
        if(countr<255)
            countr+=5;
        else if(countr==255 && countg<255)
            countg+=5;
        else if(countr==255 && countg==255 && countb<255)
            countb+=5;
        break;
    case GLUT_KEY_DOWN:
        if(countr>0)
            countr-=5;
        else if(countr==0 && countg>0)
            countg-=5;
        else if(countr==0 && countg==0 && countb>0)
            countb-=5;
        break;
    case GLUT_KEY_F11:
        //glutFullScreen();
        glutFullScreenToggle();
        break;
    }
}

//list all files in current directory
/*
void selectmusic()
{
    DIR *d;
    int i=0;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            //printf("%s\n", dir->d_name);
            instructText(dir->d_name,-250,-200+i,-500);
            i+=20;
        }

        closedir(d);
    }
}
*/
void idle()
{
    glutPostRedisplay();
}

void loadlogo()
{
    int error;
    unsigned width = 250;
    unsigned height = 250;
    const char* name = "logo.png";
    if((error=lodepng::decode(logo,width,height,name)))
    {
        printf("Error %s",lodepng_error_text(error));
        exit(0);
    }
}

int BinSrch(int freq)
{
    int i;
    if(freq<=20||freq>20000)
        return -1;
    freq-=20;
    for(i=0; i<60; i++)
    {
        if(freq>(i*333) && freq<=(i+1)*333)
            break;
    }
    return i;
}

void loadData(){
	loadlogo();
    if (!buffer.loadFromFile(fName.c_str()))
        exit(0);
    //sound.play(); called just before display
    std::cout<<"SampleRate: "<<(SAMPLE_RATE= buffer.getSampleRate())<< std::endl;
    std::cout<<"SampleCount: "<<(SAMPLE_COUNT= buffer.getSampleCount())<< std::endl;
    std::cout<<"SampleCount/2: "<<buffer.getSampleCount()/2<< std::endl;
    std::cout<<"expected no of samples: "<<(((int)(buffer.getSampleCount()/N))*N)/2<<std::endl;
    std::cout<<"channel: "<<buffer.getChannelCount()<<std::endl;
    std::cout <<"Duration: "<< buffer.getDuration().asSeconds() << " seconds"<< std::endl;

    std::vector<int>::iterator it;
    std::vector<int>::iterator f_it;
    it = ampdb.begin();
    f_it = frequency.begin();
    timestamp_t t0 = get_timestamp();

    int i;
    double mag[N / 2];
    double sf = buffer.getSampleRate();
    double roof = buffer.getSampleCount();
    double framePointer = 0;
    std::vector<double> data;
    auto array1 = buffer.getSamples();
    for (int i = 0; i < (int)buffer.getSampleCount(); i++)
    {
        data.push_back((double)array1[i]);
    }

    while (framePointer < roof  )
    {
        for ( i = framePointer, j = 0; i < (framePointer + N) && framePointer < roof - N ; i++, j++  )
        {
            //Apply Hanning window function on the sample
            double multiplier = 0.5 * (1 - cos(2 * M_PI * j / (N - 1)));
            in[j].r = multiplier * data[i];
            in[j].i = 0;  //stores N samples
        }
        if (framePointer < roof - N )
        {
            framePointer = i;
        }
        else
        {
            timestamp_t t1 = get_timestamp();
            double secs = (t1 - t0) / 1000000.0L;
            std::cout << "Total exec time: " << secs << std::endl;
            break;
        }

        // get fft values from kissfft
        getFft(in, out);

        // calculate magnitude of first n/2 FFT
        for (i = 0; i < N / 2; i++ )
        {
            int val,f;
            mag[i] = sqrt((out[i].r * out[i].r) + (out[i].i * out[i].i));
            f = (i*sf)/N;
            /*
            Frequency_Range	Frequency_Values
            Sub-bass	    20 to 60 Hz
            Bass	        60 to 250 Hz
            Low midrange	250 to 500 Hz
            Midrange	    500 Hz to 2 kHz
            Upper midrange	2 to 4 kHz
            Presence	    4 to 6 kHz
            Brilliance	    6 to 20 kHz
            */

            if (f<=60)
            {
                val = abs( (float)(log(mag[i]) * 10)/9.0);
            }
            else if (f>60 && f<=250)
            {
                val = abs((float)(log(mag[i]) * 10)/8.0);
            }
            else if (f>250 && f<=500)
            {
                val = abs((float)(log(mag[i]) * 10)/7.0);
            }
            else if (f>500 && f<=2000)
            {
                val = abs((float)(log(mag[i]) * 10)/6.0);
            }
            else if (f>2000 && f<=4000)
            {
                val = abs((float)(log(mag[i]) * 10)/5.0);
            }
            else if (f>4000 && f<=6000)
            {
                val = abs((float)(log(mag[i]) * 10)/4.0);
            }
            else if (f>6000 && f<=20000)
            {
                val = abs((float)(log(mag[i]) * 10)/3.5);
            }
            else
            {
                val = abs((float)(log(mag[i]) * 10)/2.0);
            }

            it = ampdb.end();
            f_it = frequency.end();
            it = ampdb.insert(it, val);
            f_it = frequency.insert(f_it,f);
        }

    }

    std::cout<<"actual no of samples: "<<ampdb.size();

    //frequency mapping of amplitudes
    int k=0;
    for(int i=0;i<(SAMPLE_COUNT)/(SAMPLE_RATE*0.1);i++)
    {
            array <double,60> temp={0};
            int cnt[60]={0};
            for(int j=0;j<(SAMPLE_RATE*0.1)/2;j++)
            {
                int index;
                if(k>(int)ampdb.size()){
                    break;
                }
                if((index=BinSrch(frequency[k]))!=-1)
                {
                    temp[index]+=ampdb[k];
                    cnt[index]++;
                }
                k++;
            }
            for(int j=0;j<60;j++)
            {
                temp[j]/=cnt[j];
            }
            avgarr.push_back(temp);
    }
}

void* callLoadData(void *thread)
{
	loadData();
	loading=1;
	glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    return (void*)NULL;
}

void loadingScreen()
{
    static int rot_val=0;
    drawStrokeText((char *)"Loading!",-100,125,-200);
    glColor3f(1,1,1);
    glRotated(-rot_val,0,0,1);
    float i=1.5;
    for(int theta=360;theta>0;theta-=30)
    {
        int x=60*cos(theta*3.141592/180);
        int y=60*sin(theta*3.141592/180);
        glPointSize(i);
        glBegin(GL_POINTS);
        glVertex3f(x,y,-200);
        glEnd();
        i+=.5;
    }
    rot_val+=1;
    if(rot_val>360)
        rot_val-=360;
}
void display(void)
{
    //sets color buffer bit
    glClearColor(28/255.0,49/255.0,58/255.0,0.0);
    //clears display with buffer color & depth values set in init()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Loads identity matrix for each iteration of display
    glLoadIdentity();
    if(loading==0){
    	loadingScreen();
    	if(timerFlag++==0){
            glutKeyboardFunc(NULL);
            glutSpecialFunc(NULL);
    		rc1 = pthread_create(&threads[0], NULL, callLoadData, (void *)0);
	        if (rc1){
	            cout << "Error:unable to create thread rc2," << rc1 << endl;
	            exit(-1);
	        }
    	}
    }
    else if(flag==0)
    {
        glRasterPos3f(-200,-150,-500);
        glDrawPixels(250,250,GL_RGBA,GL_UNSIGNED_BYTE,&logo[0]);
        drawStrokeText((char *)"MORPHY",-100,125,-200);
        instructText((char *)"Press space to continue!",-75,-100,-200);
        instructText((char *)"K. Vishnudev",-75,-160,-200);
        instructText((char *)"Himanshu Kumar",-75,-180,-200);
		instructText((char *)"Kartik Kamepalli",-75,-200,-200);
    }
    else
    {
        std::ostringstream ss;
        float tottime = buffer.getDuration().asSeconds();
        float cursor = sound.getPlayingOffset().asSeconds();
        curtime = sound.getPlayingOffset().asMilliseconds();
        //cout<<curtime<<endl;
        float timepercent = (cursor/tottime)*760;
        ss << (int)cursor;
        const std::string tmp = "Time : " + ss.str();
        const char* cstr = tmp.c_str();
        glClearColor(0,0,0,0);
        if(temp==1)
        {
            glColor3f(128/255.0,222/255.0,234/255.0);
            glBegin(GL_POLYGON);
            glVertex3f(-380,-290,-300.0);
            glVertex3f(-380,-280,-300.0);
            glVertex3f(-380+(int)timepercent,-280,-300.0);
            glVertex3f(-380+(int)timepercent,-290,-300.0);
            glEnd();

            glColor3f(1,1,244/255.0);
            glBegin(GL_LINE_LOOP);
            glVertex3f(-380,-292.0,-300.0);
            glVertex3f(-380,-278.0,-300.0);
            glVertex3f(380,-278.0,-300.0);
            glVertex3f(380,-292.0,-300.0);
            glEnd();


            instructText((char*)cstr,-250,-175,-200);
            nav();

            if(styleselect==0)
            {
                bars();
            }
            else if(styleselect==1)
            {
                circle3d();
            }
            else if(styleselect==2)
            {
                dust();
            }
            else if(styleselect==3)
            {
                pentagon();
            }
            else if(styleselect==4)
            {
                waves();
            }
//            else if(styleselect==4)
//            {
//                CubicalMesh();
//            }
//            else if(styleselect==6)
//            {
//                DWaves();
//            }
//            else if(styleselect==6)
//            {
//                mesh3D();
//            }
        }
        else
        {
            pausebutton();
        }
    }
    glutSwapBuffers();
}

int main(int argc, char *argv[])
{
    //SFML usage error
    if (argc < 2)
    {
        std::cout << "Usage: wave_iteration <FILENAME>" << std::endl;
        return 1;
    }

    fName=argv[1];

    glutInit(&argc, argv);
    glutSetOption(GLUT_MULTISAMPLE, 8);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH|GL_MULTISAMPLE);
    glutInitWindowSize(800,600);
    glutInitWindowPosition(0,0);
    glutCreateWindow("Morphy");
    glutKeyboardFunc( processKeys );
    glutSpecialFunc( processSpecialKeys );

    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutFullScreen();
    glutReshapeFunc(reshape);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutMainLoop();
    return 0;

}

