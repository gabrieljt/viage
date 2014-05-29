#include <VIAGE/prog.hpp>

#include <opencv2/opencv.hpp>

#include <cstdlib>
#include <cassert>
#include <iostream>


Programa::Programa(unsigned int l, unsigned int a, float t, unsigned int c, unsigned int p)
: mConect(p)
, mAGE(l, a, t, c)
{    
    std::cout << "Largura: " << l << std::endl;
    std::cout << "Altura: " << a << std::endl;
    std::cout << "Tamanho (mm): " << t << std::endl;
    std::cout << "Camera: " << c << std::endl;
    std::cout << "Porta RSI: " << p << std::endl;
}

void Programa::executar()
{
    const double setPX = -20.f;//-79.f;
    const double setPY = 200.f;//56.f;
    const double setPZ = 1100.f;

    
    while (true)
    {
        int tecla = cv::waitKey(30);
        if((tecla & 255) == 27)
            break;

        cv::Mat img;      
        if (!mAGE.filaImagens.empty())
        {          
            mAGE.mutexImagem.lock();
                img = mAGE.filaImagens.front();
                mAGE.filaImagens.pop();
            mAGE.mutexImagem.unlock();
        }

        ConectRobo::InfoRobo infoRobo;
        if (!mConect.pilhaInfoRobo.empty())
        {
            infoRobo = mConect.pilhaInfoRobo.top();
            while (!mConect.pilhaInfoRobo.empty())
                mConect.pilhaInfoRobo.pop();
            cv::putText(img, cv::format("RSolXYZ(%f,%f,%f)", infoRobo.x,infoRobo.y,infoRobo.z), 
                cv::Point(10, 425), 1, 1, cv::Scalar(255,0,255));
            cv::putText(img, cv::format("RSolABC(%f,%f,%f)", infoRobo.a,infoRobo.b,infoRobo.c), 
                cv::Point(10, 440), 1, 1, cv::Scalar(255,0,255));
        }

        if (mAGE.filaMarcadores.empty())
            mConect.RSI_XML();
        else
        {
            Marcador marco;
            mAGE.mutexMarcador.lock();
                marco = mAGE.filaMarcadores.front();
                mAGE.filaMarcadores.pop();
            mAGE.mutexMarcador.unlock();
            auto posicao = marco.getPosicao();
            auto orientacao = marco.getOrientacao();

            //ler valor vindo do robo
            double zRobo = 0.f;           

            auto x = posicao.at<double>(0, 0);
            auto y = posicao.at<double>(1, 0);
            auto z = posicao.at<double>(2, 0);
            auto a = orientacao.at<double>(0, 0);
            auto b = orientacao.at<double>(1, 0);
            auto c = orientacao.at<double>(2, 0);
            double deltaX = setPX - x;
            double deltaY = setPY - y;
            double deltaZ = setPZ - z;
            double xRSI, yRSI, zRSI;
            xRSI = yRSI = zRSI = 0.f;

            cv::putText(img, cv::format("Pos(%f,%f,%f)",x,y,z), cv::Point(10, 105), 1, 1, cv::Scalar(255,0,255));
            cv::putText(img, cv::format("Ori(%f,%f,%f)",a,b,c), cv::Point(10, 120), 1, 1, cv::Scalar(255,0,255));

            if(deltaZ < -2.5f || deltaZ > 2.5f)
            {
                xRSI = yRSI = 0.f;
                if(deltaZ > 0.f)
                {
                    cv::putText(img, cv::format("DirZ: <"), cv::Point(10, 90), 1, 1, cv::Scalar(255,0,255));
                    zRSI = -0.1f;
                }
                else if(deltaZ < 0.f)
                {
                    cv::putText(img, cv::format("DirZ: >"), cv::Point(10, 90), 1, 1, cv::Scalar(255,0,255));
                    zRSI = 0.1f;
                }
            }
            else
            {    
                zRSI = 0.f;
                cv::putText(img, cv::format("DirZ: CENTRO"), cv::Point(10, 90), 1, 1, cv::Scalar(255,0,255));

                if(deltaY < -1.f || deltaY > 1.f)
                {
                    if(deltaY > 0.f)
                    {
                        cv::putText(img, cv::format("DirY: <"), cv::Point(10, 75), 1, 1, cv::Scalar(255,0,255));
                        xRSI = -0.1f;
                    }
                    else if(deltaY < 0.f)
                    {
                        cv::putText(img, cv::format("DirY: >"), cv::Point(10, 75), 1, 1, cv::Scalar(255,0,255));
                        xRSI = 0.1f;
                    }
                }
                else
                {
                    cv::putText(img, cv::format("DirY: CENTRO"), cv::Point(10, 75), 1, 1, cv::Scalar(255,0,255));
                    xRSI = 0.f;
                }  

                if(deltaX < -1.f || deltaX > 1.f)
                {
                    if(deltaX > 0.f)
                    {
                        cv::putText(img, cv::format("DirX: <"), cv::Point(10, 60), 1, 1, cv::Scalar(255,0,255));
                        yRSI = 0.1f;
                    }
                    else if(deltaX < 0.f)
                    {
                        cv::putText(img, cv::format("DirX: >"), cv::Point(10, 60), 1, 1, cv::Scalar(255,0,255));
                        yRSI = -0.1f;
                    }
                }
                else
                {
                    cv::putText(img, cv::format("DirX: CENTRO"), cv::Point(10, 60), 1, 1, cv::Scalar(255,0,255));
                    yRSI = 0.f;
                }          
            }
            mConect.RSI_XML(xRSI, yRSI, zRSI);
            cv::putText(img, cv::format("Delta X: %f", deltaX), cv::Point(10, 15), 1, 1, cv::Scalar(255,0,255));
            cv::putText(img, cv::format("Delta Y: %f", deltaY), cv::Point(10, 30), 1, 1, cv::Scalar(255,0,255));
            cv::putText(img, cv::format("Delta Z: %f", deltaZ), cv::Point(10, 45), 1, 1, cv::Scalar(255,0,255));
        }
        if (!img.empty())
            cv::imshow("AcharTab", img);        
    }
}
