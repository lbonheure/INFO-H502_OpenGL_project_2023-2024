#version 330 core

out vec4 FragColor;

precision mediump float;

in vec4 v_col;

void main() {
    FragColor = v_col*0.2;
};