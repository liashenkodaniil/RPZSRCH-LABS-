# Лабораторна робота №3 — ROS 2 Jazzy

## Збірка (як у лабораторній №2)

```powershell
docker build -t my_ros2_jazzy_lab3 .
docker run -it -v ${PWD}:/ros2_ws my_ros2_jazzy_lab3
```

У контейнері:

```bash
cd /ros2_ws
colcon build --symlink-install
source install/setup.bash
```

Додаткові термінали: `docker ps`, потім `docker exec -it <ID> bash` і в кожному
`source /opt/ros/jazzy/setup.bash && source /ros2_ws/install/setup.bash`.

## Завдання 3.1 — робот прибирання номерів

Термінал 1 (виконавець навичок):
```bash
ros2 run lab3_hotel room_service_executor
```
Термінал 2 — дерево поведінки:
```bash
ros2 run lab3_hotel bt_orchestrator --ros-args -p season:=winter
```
або скінченний автомат SMACC2 (Ctrl+C після StMissionComplete):
```bash
ros2 run lab3_hotel_fsm sm_hotel_cleaning_node --ros-args -p season:=summer
```
Перезапускайте виконавця перед кожною місією (він "пам'ятає" стан номера).
Сценарій невдачі (3 невдалі спроби чищення):
```bash
ros2 run lab3_hotel room_service_executor --ros-args -p cleaning_failures:=5
```
Сезони: winter | summer | autumn | spring.

## Завдання 3.2 — регулятор і затримки

```bash
mkdir -p /ros2_ws/results && cd /ros2_ws/results
ros2 run lab3_control delay_sim --ros-args -p output_csv:=no_delay.csv
ros2 run lab3_control delay_sim --ros-args -p actuation_delay:=0.1 -p feedback_delay:=0.05 -p output_csv:=delay.csv
ros2 run lab3_control delay_sim --ros-args -p actuation_delay:=0.1 -p feedback_delay:=0.05 -p compensate:=true -p output_csv:=compensated.csv
ros2 run lab3_control delay_sim --ros-args -p sweep:=true
ros2 run lab3_control plot_results.py fig_compensation.png delay.csv:"Без компенсації" compensated.csv:"З предиктором" no_delay.csv:"Без затримок"
```
PNG з'явиться у папці `results` проєкту на Windows (вона змонтована в контейнер).

ros2_control (термінал 1):
```bash
ros2 launch lab3_control control.launch.py
```
Термінал 2:
```bash
ros2 control list_controllers
ros2 control list_hardware_interfaces
ros2 topic echo --once /p_controller/state
ros2 control set_controller_state p_controller_compensated active   # буде відмова: інтерфейс зайнятий
ros2 control set_controller_state p_controller inactive
ros2 control set_controller_state p_controller_compensated active
ros2 control list_controllers
ros2 topic pub --once /p_controller_compensated/target std_msgs/msg/Float64 "{data: 0.1}"
ros2 topic echo --once /p_controller_compensated/state
ros2 control switch_controllers --deactivate p_controller_compensated --activate p_controller
```
