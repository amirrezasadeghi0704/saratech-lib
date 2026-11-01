// =============================================
// مرجع کامل کتابخانه saratech_op
// Saratech Op - Complete Reference Example
//
// تمامی توابع و دستورات عمومی کتابخانه در این مثال نمایش داده شده‌اند.
// Every public function and command of the library is demonstrated here.
// =============================================

#include <saratech_op.h>

// ─── حلقه اصلی ───────────────────────────────────────────────────────────────
void setup() {
    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۱: راه‌اندازی / Initialization
    // ═══════════════════════════════════════════════════════════════════════

    // روش ساده: استفاده از تنظیمات پیش‌فرض
    // Simple method: use default settings
    saratech.begin();

    // روش پیشرفته: ارسال تنظیمات سفارشی در ساختارها
    // Advanced method: pass custom configuration structs
    SaratechOP::MotorConfig   motorCfg;
    SaratechOP::SensorConfig  sensorCfg;
    SaratechOP::ShooterConfig shooterCfg;
    SaratechOP::MotionConfig  motionCfg;

    // ── MotorConfig ────────────────────────────────────────────────────────────
    motorCfg.serial    = &Serial1;      // پورت UART موتور / Motor UART port
    motorCfg.baud      = 115200;        // نرخ baud / Baud rate
    motorCfg.dirPin    = -1;             // پین جهت RS485 (-1 = بدون پین) / RS485 DE pin (-1 = none)
    motorCfg.idFL      = 3;              // ID موتور جلو-چپ / Front-left motor ID
    motorCfg.idFR      = 0;              // ID موتور جلو-راست / Front-right motor ID
    motorCfg.idRR      = 1;              // ID موتور عقب-راست / Rear-right motor ID
    motorCfg.idRL      = 2;              // ID موتور عقب-چپ / Rear-left motor ID
    motorCfg.wheelRadius_cm = 2.5f;     // شعاع چرخ (cm) / Wheel radius (cm) — قطر 5cm تأیید شده / diameter 5cm confirmed
    motorCfg.wheelBase_cm   = 16.0f;    // فاصله محور جلو-عقب (cm) / Front-rear wheelbase
    motorCfg.track_cm       = 16.0f;    // فاصله محور چپ-راست (cm) / Left-right track width
    motorCfg.maxSpeed_cms   = 200.0f;   // حداکثر سرعت خطی (cm/s) / Max linear speed
    motorCfg.maxAccel_cms2  = 100.0f;   // حداکثر شتاب (cm/s²) / Max acceleration
    motorCfg.maxMotorRPM    = 1000.0f;  // حداکثر دور موتور (RPM) / Max motor RPM
    motorCfg.txAxisDefault  = 4000;     // زاویه پیش‌فرض TXAXIS / Default TXAXIS angle
    motorCfg.txAxisGain     = 1.0f;     // ضریb تقویت TXAXIS / TXAXIS compensation gain

    // ── SensorConfig ───────────────────────────────────────────────────────────
    sensorCfg.i2c        = &Wire2;      // I2C برای IMU / I2C for IMU
    sensorCfg.imuAddr    = 0x10;        // آدرس I2C IMU / IMU I2C address
    sensorCfg.ldrThresh  = 300;         // آستانه حساسیت LDR / LDR sensitivity threshold

    // ── ShooterConfig ──────────────────────────────────────────────────────────
    shooterCfg.triggerPin   = PC13;     // پین تریگر شوتر / Shooter trigger pin
    shooterCfg.servoPin     = PA15;     // پین سروو شوتر / Shooter servo pin
    shooterCfg.spinbackPin  = PB5;      // پین موتور اسپین‌بک / Spinback motor pin
    shooterCfg.spinbackIsPwm = true;    // استفاده از PWM برای اسپین‌بک / Use PWM for spinback
    shooterCfg.servoMinDeg  = 0.0f;     // حداقل زاویه سروو (درجه) / Min servo angle (deg)
    shooterCfg.servoMaxDeg  = 180.0f;   // حداکثر زاویه سروو (درجه) / Max servo angle (deg)
    shooterCfg.servoMinUs   = 500;      // حداقل پالس سروو (میکروثانیه) / Min servo pulse (us)
    shooterCfg.servoMaxUs   = 2400;     // حداکثر پالس سروو (میکروثانیه) / Max servo pulse (us)
    shooterCfg.spinbackMax  = 255;      // حداکثر سرعت اسپین‌بک / Max spinback speed

    // ── MotionConfig ───────────────────────────────────────────────────────────
    motionCfg.tdAxisCoeff  = 200;      // ضریب TDAXIS (0-512) / TDAXIS coefficient (0-512)
    motionCfg.yawOffsetDeg = 0.0f;     // offset عمودی اولیه / Initial yaw offset
    motionCfg.posTol_cm    = 5.0f;     // تلورانس رسیدن به هدف (cm) / Position tolerance (cm)
    motionCfg.angTol_deg   = 5.0f;     // تلورانس زاویه هدف (درجه) / Angle tolerance (deg)
    motionCfg.shiftCoeff   = 1.0f;     // ضریب پیش‌روی منحنی / Curved approach coefficient

    saratech.begin(motorCfg, sensorCfg, shooterCfg, motionCfg);

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۲: تنظیم پارامترها / Parameter Settings
    // ═══════════════════════════════════════════════════════════════════════

    // ── هندسه چرخ‌ها / Wheel Geometry ──────────────────────────────────────────
    saratech.setWheelDiameter(5.0);     // تنظیم قطر چرخ (cm) / Set wheel diameter (cm)
    saratech.setWheelRadius(2.5);       // تنظیم شعاع چرخ (cm) / Set wheel radius (cm)
    saratech.setWheelBase(16.0);        // تنظیم فاصله جلو-عقب (cm) / Set wheelbase (cm)
    saratech.setTrackWidth(16.0);       // تنظیم فاصله چپ-راست (cm) / Set track width (cm)

    // ── محدودیت‌های حرکت / Motion Limits ──────────────────────────────────────
    saratech.setMaxSpeed(200.0);        // حداکثر سرعت خطی (cm/s) / Max linear speed (cm/s)
    saratech.setMaxAccel(100.0);        // حداکثر شتاب (cm/s²) / Max acceleration (cm/s²)
    saratech.setMaxMotorRPM(1000.0);    // حداکثر دور موتور (RPM) / Max motor RPM

    // ── TDAXIS / TXAXIS ────────────────────────────────────────────────────────
    saratech.setTDAxisCoefficient(200); // تنظیم ضریب TDAXIS (0-512) / Set TDAXIS coefficient
    saratech.setTDAxis(200);             // روش کوتاه‌تر برای همان کار / Shorter alias for same
    saratech.setTXAxisCoefficient(200); // روش دیگر با نام TXAXIS / Another name alias
    saratech.setTXAxisGain(1.0f);       // ضریب تقویت جبران‌سازی / Compensation gain
    saratech.setTXAxisAngle(4000);      // زاویه خام ارسالی به GTD35 / Raw angle sent to GTD35

    // ── سنسورها / Sensors ─────────────────────────────────────────────────────
    saratech.setLDRThreshold(300);      // آستانه فعال‌سازی LDR / LDR trigger threshold
    saratech.setLDRBaseline(0, 0);      // کالیبراسیون LDR جلو / Front LDR calibration
    saratech.setLDRBaseline(1, 0);      // کالیبراسیون LDR راست / Right LDR calibration
    saratech.setLDRBaseline(2, 0);      // کالیبراسیون LDR عقب / Back LDR calibration
    saratech.setLDRBaseline(3, 0);      // کالیبراسیون LDR چپ / Left LDR calibration
    saratech.setKickerThreshold(7500.0, 90.0); // آستانه شوتر (فاصله cm، زاویه deg) / Kicker threshold
    saratech.setServoLimits(500, 2400, 0.0, 180.0); // محدودیت‌های سروو / Servo limits
    saratech.setSpinbackMaxSpeed(255);  // حداکثر سرعت اسپین‌بک / Max spinback speed
    saratech.setPositionTolerance(5.0);     // تلورانس رسیدن به نقطه هدف (cm) / Position tolerance
    saratech.setAngleTolerance(5.0);        // تلورانس رسیدن به زاویه هدف (deg) / Angle tolerance
    saratech.setShiftCoefficient(1.0);      // ضریb پیش‌روی منحنی / Shift coefficient

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۳: توابع حرکتی اصلی / Main Movement Functions
    // ═══════════════════════════════════════════════════════════════════════

    // ── Function 1: حرکت با واحدهای فیزیکی ─────────────────────────────────────
    // Move with physical units (cm/s, cm/s², degrees)
    saratech.move(100.0, 50.0, 0.0);    // جلو با 100 cm/s و شتاب 50 cm/s² / Forward at 100 cm/s, accel 50 cm/s²
    saratech.move(-80.0, 40.0, 180.0);  // عقب با 80 cm/s / Backward at 80 cm/s
    saratech.move(60.0, 30.0, 90.0);    // راست با 60 cm/s / Right at 60 cm/s
    saratech.move(60.0, 30.0, -90.0);   // چپ با 60 cm/s / Left at 60 cm/s
    saratech.move(50.0, 25.0, 45.0);    // قطری با 50 cm/s / Diagonal at 50 cm/s
    saratech.stopMotors();              // توقف همه موتورها / Stop all motors

    // ── Function 2: حرکت مختصاتی ───────────────────────────────────────────────
    // Coordinate-based movement toward a target (cm)
    bool reached = saratech.goTo(50.0, 30.0, 100.0);  // حرکت به مختصات هدف / Move to target
    if (reached) {
        saratech.stopMotors();  // توقف وقتی به هدف رسید / Stop when reached
    }

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۴: جبران‌سازی و عمودی / Compensation & Yaw
    // ═══════════════════════════════════════════════════════════════════════

    // ── Function 3: جبران‌سازی TXAXIS ─────────────────────────────────────────
    // Update TXAXIS: reads IMU and applies gyro compensation
    saratech.updateTXAXISCompensation();

    // ── Function 4A: کالیبراسیون TDAXIS ───────────────────────────────────────
    // Set TDAXIS coefficient (scales gyro correction strength)
    saratech.setTDAxis(200);           // ضریب 200 (0-512) / Coefficient 200 (0-512)

    // ── Function 4B: چرخش عمدی عمودی ─────────────────────────────────────────
    // Intentional yaw rotation
    saratech.rotateYaw(5.0);           // 5 درجه به راست (CW) / 5° right turn
    saratech.rotateYaw(-10.0);         // 10 درجه به چپ (CCW) / 10° left turn
    saratech.resetYaw();               // بازگشت به مرجع اصلی / Reset to original reference

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۵: شوتر / Shooter
    // ═══════════════════════════════════════════════════════════════════════

    // ── Function 4C: شوتر ─────────────────────────────────────────────────────
    // Shoot with servo angle and spinback speed
    saratech.shoot(90.0, 200);         // شوت مستقیم با اسپین‌بک 200 / Straight shot with spinback 200
    saratech.shoot(45.0, 150);         // شوت کج با اسپین‌بک متوسط / Angled shot, medium spinback
    saratech.shoot(0.0, 0);            // بستن شوتر / Close shooter
    saratech.shoot(180.0, 255);        // شوت باز با حداکثر اسپین‌بک / Full open with max spinback

    // کنترل جداگانه سروو و اسپین‌بک
    // Separate servo and spinback control
    saratech.setShooterServoAngle(90.0);  // تنظیم زاویه سروو / Set servo angle
    saratech.setSpinbackSpeed(128);       // تنظیم سرعت اسپین‌بک / Set spinback speed

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۶: پیش‌روی منحنی / Curved Approach (Shift)
    // ═══════════════════════════════════════════════════════════════════════

    // ── Function 5: Shift ──────────────────────────────────────────────────────
    // Calculate curved approach heading angle
    float shiftResult = saratech.shift(
        saratech.getRobotX(),     // موقعیت X فعلی ربات (cm) / Current robot X (cm)
        saratech.getRobotY(),     // موقعیت Y فعلی ربات (cm) / Current robot Y (cm)
        100.0,                    // موقعیت X توپ (cm) / Ball X position (cm)
        50.0,                     // موقعیت Y توپ (cm) / Ball Y position (cm)
        1.0                       // ضریب منحنی / Curvature coefficient
    );
    // استفاده از زاویه محاسبه‌شده
    // Use the calculated angle
    saratech.move(100.0, 50.0, shiftResult);

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۷: خواندن سنسورها / Reading Sensors
    // ═══════════════════════════════════════════════════════════════════════

    // بروزرسانی همه سنسورها
    // Update all sensors
    saratech.updateSensors();

    // هر سنسور به‌طور جداگانه
    // Each sensor individually
    saratech.updateIMU();         // خواندن زاویه ژیروسکوپ / Read gyroscope yaw
    saratech.updateLDR();         // خواندن ۴ جهت نوری / Read 4-direction LDRs
    saratech.updateAnalogSensors(); // خواندن سنسورهای آنالوگ / Read analog sensors
    saratech.updateKicker();      // بررسی وضعیت شوتر / Check kicker state

    // دریافت مقادیر خوانده‌شده
    // Get read values
    float robotYaw      = saratech.getRobotYaw();       // زاویه عمودی (درجه) / Robot yaw (deg)
    float robotYawRaw   = saratech.getRobotYawRaw();    // زاویه خام ژیروسکوپ / Raw gyro angle
    float robotX        = saratech.getRobotX();         // موقعیت X ربات (cm) / Robot X position
    float robotY        = saratech.getRobotY();         // موقعیت Y ربات (cm) / Robot Y position
    bool  inKicker      = saratech.isBallInKicker();    // آیا توپ در شوتر است؟ / Is ball in kicker?
    uint16_t tdAxisVal  = saratech.getTDAxisCoeff();    // ضریb فعلی TDAXIS / Current TDAXIS coeff
    float  yawOff       = saratech.getYawOffset();      // offset عمودی فعلی / Current yaw offset
    float  maxSpd       = saratech.getMaxSpeed();       // حداکثر سرعت تنظیم‌شده / Max configured speed
    float  maxAcc       = saratech.getMaxAccel();       // حداکثر شتاب تنظیم‌شده / Max configured acceleration
    float  wRad         = saratech.getWheelRadius();    // شعاع چرخ فعلی (cm) / Current wheel radius
    float  wBase        = saratech.getWheelBase();      // فاصله محور چرخ‌ها (cm) / Wheelbase distance
    float  trackW       = saratech.getTrackWidth();     // عرض محور چرخ‌ها (cm) / Track width

    // وضعیت LDRها
    // LDR status
    bool ldrFront = saratech.getLDRFront();
    bool ldrRight = saratech.getLDRRight();
    bool ldrBack  = saratech.getLDRBack();
    bool ldrLeft  = saratech.getLDRLeft();

    // مقادیر سنسورهای آنالوگ
    // Analog sensor values
    int bottomSensor = saratech.getBottomSensor();
    int rightSensor  = saratech.getRightSensor();
    int leftSensor   = saratech.getLeftSensor();

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۸: کنترل مستقیم موتورها / Direct Motor Control
    // ═══════════════════════════════════════════════════════════════════════

    // کنترل با مقادیر مستقیم (signed int16)
    // Direct control with signed int16 values
    saratech.setMotorSpeeds(1000, 1000, 1000, 1000);  // FL, FR, RR, RL
    saratech.setMotorSpeeds(-500, -500, -500, -500);  // معکوس / Reverse
    saratech.setMotorSpeeds(0, 0, 0, 0);              // توقف / Stop

    // کنترل بر حسب RPM
    // Control by RPM
    saratech.setMotorSpeedsRPM(50.0, 50.0, 50.0, 50.0);  // 50 RPM همه موتورها / 50 RPM all motors
    saratech.setMotorSpeedsRPM(0.0, 0.0, 0.0, 0.0);      // توقف / Stop

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۹: دسترسی به سخت‌افزار / Hardware Access
    // ═══════════════════════════════════════════════════════════════════════

    // دسترسی مستقیم به GTD35 (مدیریت موتور)
    // Direct GTD35 access (motor management)
    GTD35& gtd = saratech.gtd35();
    gtd.motor(100, 100, 100, 100);        // ارسال فرمان مستقیم / Send direct command
    gtd.setEnabled(false);                // غیرفعال کردن موتور / Disable motor
    gtd.setEnabled(true);                 // فعال‌سازی موتور / Enable motor
    gtd.setAngle(4000);                   // تنظیم TXAXIS مستقیم / Set TXAXIS directly
    gtd.setMotorIDs(3, 0, 1, 2);          // تغییر IDs موتورها / Change motor IDs

    // دسترسی مستقیم به IMU (TDAxis12)
    // Direct IMU access (TDAxis12)
    TDAxis12& imuDev = saratech.imu();
    float gyroVal = imuDev.read();        // خواندن زاویه / Read angle

    // دسترسی مستقیم به کنترل‌کننده سروو
    // Direct servo controller access
    STM32_ISR_Servo& srv = saratech.servoCtrl();
    srv.setPosition(0, 90.0);             // تنظیم سروو 0 روی 90 درجه / Set servo 0 to 90 deg
    int pos = srv.getPosition(0);         // خواندن موقعیت سروو / Read servo position

    // دسترسی مستقیم به OLED (SH1106)
    // Direct OLED access (SH1106)
    Adafruit_SH1106& disp = saratech.display();
    disp.clearDisplay();                  // پاک کردن صفحه / Clear display
    disp.setCursor(0, 0);                 // تنظیم مکان‌نما / Set cursor position
    disp.setTextSize(1);                  // اندازه متن / Text size
    disp.setTextColor(WHITE);             // رنگ متن / Text color
    disp.print("Saratech OP");            // چاپ متن / Print text
    disp.println("v1.0");                 // چاپ با خط جدید / Print with newline
    disp.drawCircle(64, 32, 20, WHITE);   // رسم دایره / Draw circle
    disp.drawLine(0, 0, 127, 63, WHITE); // رسم خط / Draw line
    disp.display();                       // بروزرسانی فیزیکی صفحه / Actually update display

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۱۰: نمایش و دیباگ / Display & Debug
    // ═══════════════════════════════════════════════════════════════════════

    // چاپ اطلاعات روی Serial و OLED
    // Print debug info to both Serial and OLED
    saratech.printDebug();

    // ═══════════════════════════════════════════════════════════════════════
    // بخش ۱۱: تست کامل حرکت و بازگشت به نقطه شروع
    // Section 11: Complete movement test and return to start
    // ═══════════════════════════════════════════════════════════════════════

    // نمونه عملی: حرکت به یک مختصات با shift
    // Practical example: move to coordinate with curved approach
    float shiftAngle = saratech.shift(
        saratech.getRobotX(),
        saratech.getRobotY(),
        100.0f,
        0.0f,
        1.0f);     // ضریb شیفت / Shift coefficient

    saratech.move(100.0f, 50.0f, shiftAngle);
}

void loop() {
    // ═══════════════════════════════════════════════════════════════════════
    // حلقه اصلی: خواندن سنسور + حرکت خودکار
    // Main loop: sensor reading + autonomous movement
    // ═══════════════════════════════════════════════════════════════════════

    // بروزرسانی سنسورها
    // Update sensors
    saratech.updateSensors();

    // حرکت بر اساس LDR
    // Move based on LDR inputs
    if (saratech.getLDRFront()) {
        saratech.move(80.0, 40.0, 0.0);     // جلو / Forward
    } else if (saratech.getLDRRight()) {
        saratech.move(80.0, 40.0, 90.0);    // راست / Right
    } else if (saratech.getLDRLeft()) {
        saratech.move(80.0, 40.0, -90.0);   // چپ / Left
    } else if (saratech.getLDRBack()) {
        saratech.move(80.0, 40.0, 180.0);   // عقب / Backward
    } else {
        saratech.stopMotors();               // توقف / Stop
    }

    // به‌روزرسانی جبران‌سازی
    // Update compensation
    saratech.updateTXAXISCompensation();

    // نمایش اطلاعات
    // Display debug info
    saratech.printDebug();
}
