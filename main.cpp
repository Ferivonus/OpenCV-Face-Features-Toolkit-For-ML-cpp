#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <unordered_map>

cv::CascadeClassifier face_cascade;
cv::Mat frame, frame_gray;

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define platform_mkdir(directory) _mkdir(directory)
#else
#include <sys/stat.h>
#define platform_mkdir(directory) mkdir(directory, 0777)
#endif

void createDirectory(const std::string& dirName) {
#if defined(_WIN32) || defined(_WIN64)
    int result = platform_mkdir(dirName.c_str());
#else
    int result = platform_mkdir(dirName.c_str(), 0777);  // For Unix-like systems
#endif

    if (result != 0) {
        std::cerr << "Failed to create directory: " << dirName << std::endl;
    }
}
bool isEyeClosed(const cv::Mat& eyeROI) {
    cv::Scalar meanIntensity = cv::mean(eyeROI);
    double threshold = 80; // Gözün kapalı kabul edilmesi için eşik değeri
    return meanIntensity[0] < threshold;
}

void saveFacesFromVideo(const std::string& videoPath, const std::string& outputDir, int minFaceCount, int frameSkip, int maxFaceCount) {
    cv::VideoCapture videoCapture(videoPath);

    if (!videoCapture.isOpened()) {
        std::cerr << "Video cannot be opened." << std::endl;
        return;
    }

    int totalFaceCount = 0;

    std::string videoNameWithExtension = videoPath.substr(videoPath.find_last_of("/\\") + 1);
    std::string videoName = videoNameWithExtension.substr(0, videoNameWithExtension.find_last_of("."));
    std::string videoOutputDir = outputDir + "/" + videoName;
    createDirectory(videoOutputDir);

    cv::CascadeClassifier eye_cascade;
    if (!eye_cascade.load("C:\\Program Files\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml")) {
        std::cout << "Failed to load eye detection classifiers." << std::endl;
        return;
    }

    while (totalFaceCount < maxFaceCount) {
        for (int i = 0; i < frameSkip; ++i) {
            videoCapture >> frame;
            if (frame.empty()) {
                break;
            }
        }

        if (frame.empty()) {
            break;
        }

        cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        for (const cv::Rect& faceRect : faces) {
            cv::Mat faceROI = frame_gray(faceRect);
            std::vector<cv::Rect> eyes;
            eye_cascade.detectMultiScale(faceROI, eyes);

            if (eyes.size() >= 2) {
                bool allEyesOpen = true;
                for (const cv::Rect& eyeRect : eyes) {
                    cv::Mat eyeROI = faceROI(eyeRect);
                    if (isEyeClosed(eyeROI)) {
                        allEyesOpen = false;
                        break;
                    }
                }

                if (allEyesOpen) {
                    std::string faceFilename = videoOutputDir + "/" + videoName + "_face_" + std::to_string(totalFaceCount) + ".jpg";
                    cv::Mat faceImage = frame(faceRect);
                    cv::imwrite(faceFilename, faceImage);
                    ++totalFaceCount;
                }
            }
        }

        cv::imshow("Saving Faces from Video", frame);
        if (cv::waitKey(1) == 27 || totalFaceCount >= maxFaceCount) {
            break;
        }
    }

    videoCapture.release();
    cv::destroyAllWindows();
}

void CameraVideoCapture()
{
    // Load face detection classifiers:
    if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
        std::cout << "Failed to load face detection classifiers." << std::endl;
        return;
    }

    cv::VideoCapture cap;
    // Open the camera:
    cap.open(0);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera." << std::endl;
        return;
    }

    while (true) {
        // Capture each frame:
        cap >> frame;

        // Convert the frame to grayscale:
        cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

        // Detect face features:
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Draw a rectangle for each face:
        for (size_t i = 0; i < faces.size(); i++) {
            cv::rectangle(frame, faces[i], cv::Scalar(0, 255, 0), 2);
        }

        // Display the frame:
        cv::imshow("Detecting Face Features", frame);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}

void ScreenCapture()
{
    // Load face detection classifiers:
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
        std::cout << "Failed to load face detection classifiers." << std::endl;
        return;
    }

    // Define a variable for capturing the screen:
    cv::Mat screen;

    while (true) {
        // Capture the screen:
        HDC hDCDesktop = GetDC(NULL);
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        HDC hDCMemDC = CreateCompatibleDC(hDCDesktop);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDCDesktop, nScreenWidth, nScreenHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDCMemDC, hBitmap);
        BitBlt(hDCMemDC, 0, 0, nScreenWidth, nScreenHeight, hDCDesktop, 0, 0, SRCCOPY);
        screen = cv::Mat(nScreenHeight, nScreenWidth, CV_8UC4);
        GetBitmapBits(hBitmap, nScreenHeight * nScreenWidth * 4, screen.data);
        cv::cvtColor(screen, screen, cv::COLOR_BGRA2BGR);
        ReleaseDC(NULL, hDCDesktop);
        DeleteDC(hDCMemDC);
        DeleteObject(hBitmap);

        // Detect face features:
        std::vector<cv::Rect> faces;
        cv::Mat grayFrame;
        cv::cvtColor(screen, grayFrame, cv::COLOR_BGR2GRAY);
        face_cascade.detectMultiScale(grayFrame, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Draw a rectangle for each face:
        for (size_t i = 0; i < faces.size(); i++) {
            cv::rectangle(screen, faces[i], cv::Scalar(0, 255, 0), 2);
        }

        // Display the frame:
        cv::imshow("Detecting Face Features", screen);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}

void FindAndHighlightImage() {
    // Load the image to search for:
    std::string image_path = "pictures\\discord pp 2.jpg"; // Replace with the actual path to the image
    cv::Mat search_image = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    if (search_image.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return;
    }

    // Define a variable for capturing the screen:
    cv::Mat screen;

    while (true) {
        // Capture the screen:
        HDC hDCDesktop = GetDC(NULL);
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        HDC hDCMemDC = CreateCompatibleDC(hDCDesktop);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDCDesktop, nScreenWidth, nScreenHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDCMemDC, hBitmap);
        BitBlt(hDCMemDC, 0, 0, nScreenWidth, nScreenHeight, hDCDesktop, 0, 0, SRCCOPY);
        screen = cv::Mat(nScreenHeight, nScreenWidth, CV_8UC4);
        GetBitmapBits(hBitmap, nScreenHeight * nScreenWidth * 4, screen.data);
        cv::cvtColor(screen, screen, cv::COLOR_BGRA2BGR);
        ReleaseDC(NULL, hDCDesktop);
        DeleteDC(hDCMemDC);
        DeleteObject(hBitmap);

        // Match the template image with the captured screen:
        cv::Mat result;
        cv::matchTemplate(screen, search_image, result, cv::TM_CCOEFF_NORMED);

        // Define the threshold value for matching:
        double threshold = 0.8;

        // Get the location of the best match:
        cv::Point max_loc;
        cv::minMaxLoc(result, NULL, NULL, NULL, &max_loc);

        // If the match is good enough, draw a green rectangle and circle around it:
        if (result.at<float>(max_loc.y, max_loc.x) >= threshold) {
            cv::Rect rect(max_loc.x, max_loc.y, search_image.cols, search_image.rows);
            cv::rectangle(screen, rect, cv::Scalar(0, 255, 0), 2);
            cv::circle(screen, cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2), max(rect.width, rect.height) / 2, cv::Scalar(0, 255, 0), 2);
        }


        // Display the frame:
        cv::imshow("Find and Highlight Image", screen);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}

void CameraVideoCaptureBlurFace()
{
    // Load face detection classifiers:
    if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
        std::cout << "Failed to load face detection classifiers." << std::endl;
        return;
    }

    cv::VideoCapture cap;
    // Open the camera:
    cap.open(0);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera." << std::endl;
        return;
    }

    while (true) {
        // Capture each frame:
        cap >> frame;

        // Convert the frame to grayscale:
        cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

        // Detect face features:
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Blurring each detected face:
        for (size_t i = 0; i < faces.size(); i++) {
            cv::Mat face = frame(faces[i]);
            cv::blur(face, face, cv::Size(25, 25));
        }

        // Display the frame:
        cv::imshow("Detecting and Blurring Face Features", frame);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}

void ScreenCaptureBlurFace()
{
    // Load face detection classifiers:
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
        std::cout << "Failed to load face detection classifiers." << std::endl;
        return;
    }

    // Define a variable for capturing the screen:
    cv::Mat screen;

    while (true) {
        // Capture the screen:
        HDC hDCDesktop = GetDC(NULL);
        int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        HDC hDCMemDC = CreateCompatibleDC(hDCDesktop);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDCDesktop, nScreenWidth, nScreenHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDCMemDC, hBitmap);
        BitBlt(hDCMemDC, 0, 0, nScreenWidth, nScreenHeight, hDCDesktop, 0, 0, SRCCOPY);
        screen = cv::Mat(nScreenHeight, nScreenWidth, CV_8UC4);
        GetBitmapBits(hBitmap, nScreenHeight * nScreenWidth * 4, screen.data);
        cv::cvtColor(screen, screen, cv::COLOR_BGRA2BGR);
        ReleaseDC(NULL, hDCDesktop);
        DeleteDC(hDCMemDC);
        DeleteObject(hBitmap);

        // Detect face features:
        std::vector<cv::Rect> faces;
        cv::Mat grayFrame;
        cv::cvtColor(screen, grayFrame, cv::COLOR_BGR2GRAY);
        face_cascade.detectMultiScale(grayFrame, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Blurring each detected face:
        for (size_t i = 0; i < faces.size(); i++) {
            cv::Mat face = screen(faces[i]);
            cv::blur(face, face, cv::Size(25, 25));
        }

        // Display the frame:
        cv::imshow("Screen Capture with Blurred Faces", screen);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}

void CameraVideoCaptureFilterFace()
{
    // Load face detection classifiers:
    if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
        std::cout << "Failed to load face detection classifiers." << std::endl;
        return;
    }

    cv::VideoCapture cap;
    // Open the camera:
    cap.open(0);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera." << std::endl;
        return;
    }

    while (true) {
        // Capture each frame:
        cap >> frame;

        // Detect face features:
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(frame, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Apply filter on each detected face:
        for (size_t i = 0; i < faces.size(); i++) {
            cv::Mat face = frame(faces[i]);
            cv::Mat filtered_face;
            // Define your desired filter:
            cv::GaussianBlur(face, filtered_face, cv::Size(25, 25), 0);
            filtered_face.copyTo(face);
        }

        // Display the frame:
        cv::imshow("Detecting and Filtering Face Features", frame);

        // Exit the loop if ESC key is pressed:
        if (cv::waitKey(1) == 27) {
            break;
        }
    }
}






int main() {
    int choice;

    std::cout << "What operation would you like to perform?\n1: Detecting Face Features on Camera Video Capture\n2: Detecting Face Features on Computer Screen Capture\n3: Find and Highlight Image\n4: Detecting and Blurring Face Features on Camera Video Capture\n5: Detecting and Blurring Face Features on Computer Screen Capture\n6: Camera capture filite\n7: Dedecting and Saving Faces from a spesific MP4 file\n";
    std::cin >> choice;

    if (choice == 1) {
        CameraVideoCapture();
    }
    else if (choice == 2) {
        ScreenCapture();
    }
    else if (choice == 3) {
        FindAndHighlightImage();
    }
    else if (choice == 4) {
        CameraVideoCaptureBlurFace();
    }
    else if (choice == 5) {
        ScreenCaptureBlurFace();
    }
    else if (choice == 6) {
        CameraVideoCaptureFilterFace();
    }
    else if (choice == 7) {
        std::vector<std::string> videoPaths = {
            //Youtube videos:
            "C:\\Users\\fahre\\Pictures\\Camera Roll\\example.mp4"
        
        // Add more video paths as needed
        };

        std::string outputDir = "output_faces";  // Main output directory

        int minFaceCount = 20;
        int frameSkip = 5;
        int maxFaceCount = 50;

        // Load face detection classifiers:
        if (!face_cascade.load("C:\\Program Files\\opencv\\extras\\haarcascade_frontalface_alt.xml")) {
            std::cout << "Failed to load face detection classifiers." << std::endl;
            return -1;
        }
        createDirectory(outputDir);

        for (const std::string& videoPath : videoPaths) {
            saveFacesFromVideo(videoPath, outputDir, minFaceCount, frameSkip, maxFaceCount);
            std::cout << "JPEGs created for video: " << videoPath << std::endl;
        }

        return 0;
    }
    else {
        std::cout << "Invalid choice!" << std::endl;
        return -1;
    }

    return 0;
}

